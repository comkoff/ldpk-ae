# 3DE4 Lens Distortion для After Effects

**Язык:** [English](readme.md) | Русский

Мост между LDPK (Lens Distortion Plugin Kit, Science-D-Visions) и After Effects:
эффект читает `.nk`-файл с нодой линзовой дисторсии, экспортированной из 3DE4
скриптом `ldpk-2.14.0/3de4/python/export_nuke_LD_3DE4_Lens_Distortion_Node.py`, и применяет
дисторсию к слою, используя header-only модели LDPK.

## Состав

| Файл | Назначение |
|---|---|
| [`ldpk_nk_block.h`](ldpk_nk_block.h) | Парсер блока `LD_3DE4_* { ... }` из `.nk`: статические параметры и кривые `{{curve x<frame> <value>}}` с линейной интерполяцией |
| [`ldpk_model_registry.h`](ldpk_model_registry.h) | Соответствие «имя ноды Nuke → класс LDPK» (все актуальные модели 3DE4) |
| [`ldpk_nk_loader.h`](ldpk_nk_loader.h) | Загрузка модели из файла: 7 встроенных параметров + параметры модели + FOV-маппинг (как в `nuke_ld_3de4_base.C`) |
| [`nk_ld_dump.cpp`](nk_ld_dump.cpp) | Консольная утилита: печатает параметры и сетку координат — для сверки с Nuke |
| [`test_snapshot_roundtrip.cpp`](test_snapshot_roundtrip.cpp) | Тест: сериализация снимка .nk (round-trip), сканер блока в полном скрипте, согласованность `apply`/`apply_checked` |
| [`ae/LD3DE4_Lens_Distortion.h`](ae/LD3DE4_Lens_Distortion.h) / [`.cpp`](ae/LD3DE4_Lens_Distortion.cpp) | Эффект After Effects (AEX) |
| [`ae/LD3DE4_Lens_DistortionPiPL.r`](ae/LD3DE4_Lens_DistortionPiPL.r) | PiPL-ресурс плагина |
| [`ae/Win/LD3DE4_Lens_Distortion.sln`](ae/Win/LD3DE4_Lens_Distortion.sln) / [`.vcxproj`](ae/Win/LD3DE4_Lens_Distortion.vcxproj) | Проект Visual Studio 2022 (v143, x64) |

## Сборка

Требуется Visual Studio 2022 (toolset v143) с workload «Desktop development with C++».
SDK лежит в `AfterEffectsSDK_26.5_win`.

```cmd
:: консольная утилита
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cd ae_ldpk
cl /nologo /EHsc /MD /D_WINDOWS /I..\ldpk-2.14.0\include nk_ld_dump.cpp /Fe:nk_ld_dump.exe

:: плагин AE
set AE_PLUGIN_BUILD_DIR=%TEMP%\ae_ldpk_plugins
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" ^
  ae_ldpk\ae\Win\LD3DE4_Lens_Distortion.sln /p:Configuration=Release /p:Platform=x64

:: диагностическая сборка (журнал %TEMP%\ld3de4_ae.log всегда включён)
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" ^
  ae_ldpk\ae\Win\LD3DE4_Lens_Distortion.sln /p:Configuration=Release /p:Platform=x64 /p:LD_DEBUG_LOG=1
```

Диагностическая сборка получает отдельные имя («3DE4 Lens Distortion
Debug») и Match Name (`SDV LD_3DE4 Lens Distortion Debug`) — обе версии
можно держать в Plug-ins одновременно.

Готовый плагин: `releases/AE_LD3DE4/<версия>/LD_3DE4_Lens_Distortion_v<версия>.aex`
(OutDir проекта) — скопируйте в
`C:\Program Files\Adobe\Adobe After Effects <версия>\Support Files\Plug-ins\`.

## Использование в AE

1. Примените эффект **Lens Distortion → 3DE4 Lens Distortion** к слою.
2. Выберите модель в выпадающем списке **Model** — эффект работает сразу,
   без .nk файла (параметры модели и FOV = [0..1]² — дефолты LDPK).
3. Нажмите **Choose .nk File...** и выберите файл, экспортированный из 3DE4
   (или содержащий ноду `LD_3DE4_*` — блок находится и в полном .nk-скрипте
   Nuke): popup Model переключится на модель из файла, статичные параметры
   заполнятся, а кривые .nk автоматически расставятся как ключи (LINEAR,
   весь диапазон кривой, без смещения).
   Файл разбирается **до** применения: при ошибке состояние эффекта не
   меняется. Успешный импорт сохраняет **снимок** разобранной ноды внутри
   проекта — файл больше не нужен (см. «Снимок .nk» ниже).
4. **Direction**: `Undistort` (убрать дисторсию, как в Nuke-ноде по умолчанию)
   или `Distort` (вернуть).
5. **Fit Comp To Distortion...**: точно подгоняет холст композиции
   под изображение после применения эффекта — увеличивает, когда дисторсия
   выходит за кадр, и уменьшает, когда искажённое изображение занимает меньше
   пикселей, чем кадр. Слои не трогаются (пиксели остаются 1:1, кадр
   «прирастает» или «сжимается» полями). При анимированной дисторсии (ключи
   на слайдерах или кривые в .nk) холст подгоняется под объединение границ
   по всем ключевым кадрам — самый широкий образ по времени. Учитывается
   Scale слоя (значение на текущем кадре): эффект рендерится в layer space,
   AE применяет трансформы слоя после эффектов, поэтому уменьшенный без
   прекомпоза слой даёт пропорционально меньший прирост компа. Анимация
   Scale не сканируется — размер компа в AE анимировать нельзя.

### Буфер и охват

- Выходной буфер эффекта **всегда** расширяется до искажённых границ кадра
  (LDPK `getBoundingBox*`; smart render: PreRender сообщает
  `result_rect`/`max_result_rect` = кадр + расширение).
- AE обрезает расширенный буфер рамкой композиции, поэтому до нажатия
  **Fit Comp To Distortion...** картинка выглядит как «Original Frame».
- После нажатия кнопки комп становится ровно размером с буфер (без запаса) —
  полный undistorted-образ виден целиком, лишних пикселей нет.
- При анимации Fit Comp сканирует все ключи слайдеров эффекта и кривых .nk
  и подгоняет комп под объединение искажённых границ (до 2048 точек скана).
- Scale слоя учитывается по значению на текущем кадре (без скана ключей:
  размер компа анимировать нельзя). Требуемый размер компа умножается на
  Scale слоя; Scale 0% — комп не меняется.
- Размер компа подгоняется точно по каждой стороне (без пропорционального
  масштабирования). При нулевых
  коэффициентах кнопка возвращает комп к размеру слоя с учётом Scale;
  FP-шум bbox (< 1e-6 в unit-координатах) отсекается, чтобы тождественная
  модель давала ровно исходный размер без лишнего пикселя.
- Ограничение: сам выходной буфер рендера расширяется по текущему кадру —
  на кадрах с меньшей дисторсией края компа будут прозрачными (изображения
  там нет), это корректно.

## Координаты

- LDPK/Nuke: unit-координаты, (0,0) — левый нижний угол, y вверх.
- AE: пиксели, (0,0) — левый верхний угол, y вниз.
- Преобразование выполняется в пиксельных колбэках
  [`LD_RenderPixel8/16/Float`](ae/LD3DE4_Lens_Distortion.cpp):
  `uy = 1 - oy/h` и обратно `py = (1-qy)*h - 0.5`.
- Сэмплинг — субпиксельный (`subpixel_sample` / `subpixel_sample16` /
  `subpixel_sample_float`), как в SDK-примере Shifter.

## Поддерживаемые модели

В UI (popup Model и группы параметров) — 10 моделей, соответствующие
встроенному набору старого и нового 3DE:

`LD_3DE_Classic_LD_Model`, `LD_3DE4_Radial_Standard_Degree_4`,
`LD_3DE4_Radial_Stabilizer_Degree_6`, `LD_3DE4_Anamorphic_Degree_6`,
`LD_3DE4_Anamorphic_Standard_Degree_4/6`, `LD_3DE4_Anamorphic_Rescaled_Degree_4/6`,
`LD_3DE4_Anamorphic_Stabilizer_Degree_6`, `LD_3DE4_Radial_Fisheye_Degree_8`.

Скрыты от UI (флаг `ui_visible = false` в [`ldpk_param_tables.h`](ldpk_param_tables.h)),
но остаются в реестре и загружаются из `.nk`-файлов: `LD_3DE4_Brown_Conrady_Degree_6_2`,
`LD_3DE4_Radial_Fisheye_{Equidistant,Equisolid,Orthographic,Stereographic}_Degree_8`
(experimental), `LD_3DE4_{Radial,Anamorphic}_Homomorphic_Degree_2` (toymodels),
`LD_OpenCV_Radial_Degree_6_Decentered_Degree_2`. Если такой класс встретится в
`.nk`, эффект отрендерит его параметрами из файла; popup Model при этом не
переключается.

Не поддерживаются (устаревшие/несовместимые с MSVC): `LD_3DE4_Noop_Debugging`
(массив нулевой длины — C2466), `LD_3DE4_Radial_Fisheye_Degree_8_Release_1`.

## Снимок .nk

- При импорте разобранная нода сериализуется в sequence data эффекта
  (бинарный блоб, ~1 КБ на статичную ноду, десятки КБ при анимации).
  Проект **самодостаточен**: рендер не обращается к .nk-файлу, перенос
  проекта и рендер на ферме дают ту же геометрию, пропажа файла не меняет
  картинку.
- Снимок — истина; обновление линзы —
  повторный импорт файла (кнопка или Options). Повторный импорт заменяет
  старые ключи новыми; параметр, отсутствующий в новом файле, сбрасывается
  к дефолту таблицы, анимация снимается. Options-кнопка идёт по тому же
  пути, что и кнопка выбора файла (с запеканием).
- Единый источник модели для рендера, границ буфера и Fit Comp
  (`LD_PrepareModel`): класс — из popup, если класс снимка виден в UI,
  иначе из снимка (скрытые классы); коэффициенты — из группы выбранной
  модели (видимые) или из снимка (скрытые, слайдеры не проталкиваются);
  FOV и 7 встроенных параметров камеры — из снимка, без снимка — дефолты
  LDPK. Смена Model после импорта переключает модель; коэффициенты берутся
  из группы выбранной модели; FOV и параметры камеры — из .nk.
- Запекание выполняется только когда класс снимка совпадает с popup и
  видим в UI; для скрытых классов — пропуск с записью в лог.
- Время ключа при запекании — через точную длительность кадра компа
  (`AEGP_GetCompFrameDuration`), без округления fps.

## Ограничения

- Кривые параметров интерполируются линейно (экспортёр 3DE4 пишет ключи без
  сплайновых хендлов; Nuke по умолчанию тоже использует linear для таких кривых).
- Анимация FOV не поддерживается (FOV в экспорте статичен; без .nk FOV = [0..1]²).
  Анимированные встроенные параметры камеры (например, фокус fisheye)
  поддерживаются — снимок хранит их кривые.
- Пиксельный аспект композиции предполагается равным 1.
- Рендер — smart render (`SMART_PRE_RENDER`/`SMART_RENDER`): 8/16/32 bpc
  поддерживаются нативно, формат определяется по выданным мирам
  (`PF_GetPixelFormat`), 32 bpc — через float-сэмплинг. AE требует пару
  флагов `SUPPORTS_SMART_RENDER` + `FLOAT_COLOR_AWARE` (иначе 25::248).
- Fit Comp учитывает Scale слоя на текущем кадре; поворот слоя и положение
  слоя не по центру компа не учитываются.
- Для точной сверки с Nuke используйте `nk_ld_dump.exe` и тестовые проекты
  из `ldpk-2.14.0/projects/nuke/`.

## Лицензия

Код моста — «as is», без гарантий. Модели LDPK — free software
(см. `license` в корне LDPK); заголовок `tde4_ld_plugin.h` принадлежит
Science-D-Visions. Примеры Adobe SDK используются как структурный референс
(Resizer, Shifter, PathMaster, Gamma_Table).
