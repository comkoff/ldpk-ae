# ldpk-ae — линзовая дисторсия LDPK → After Effects / Nuke

Инструменты линзовой дисторсии на базе LDPK 2.14.0 (Lens Distortion Plugin Kit, Science-D-Visions).

**Язык:** [English](README.md) | Русский

Главный компонент — эффект **After Effects «3DE4 Lens Distortion»**: применяет к слою дисторсию, посчитанную в 3DE4 — по `.nk`-файлу, экспортированному из 3DE4 (File → Export → Export Nuke LD_3DE4 Lens Distortion Node), либо по параметрам, заданным вруч<arg_value>ную. Поддерживаются анимированные параметры (кривые 3DE4 → keyframes), Undistort/Distort, автоподгонка композиции (Fit Comp), 8/16/32 bpc.

## Две сборки: release и debug

Плагин собирается в **двух версиях**, которые можно держать в Plug-ins одновременно:

| Сборка | Пункт меню | Match Name | Журнал |
|---|---|---|---|
| **Release** | Lens Distortion → 3DE4 Lens Distortion | `SDV LD_3DE4 Lens Distortion` | не пишется |
| **Debug** (диагностическая) | Lens Distortion → 3DE4 Lens Distortion Debug | `SDV LD_3DE4 Lens Distortion Debug` | всегда пишет `%TEMP%\ld3de4_ae.log` (ротация при 10 МБ, бэкап `ld3de4_ae.log.1`) |

Для поиска проблем используйте диагностическую сборку: примените её к копии композиции, воспроизведите проблему и приложите журнал.

## Состав репозитория

| Каталог | Содержимое |
|---|---|
| [`ae_ldpk/`](ae_ldpk) | Мост LDPK → After Effects: парсер `.nk` ([`ldpk_nk_block.h`](ae_ldpk/ldpk_nk_block.h)), реестр моделей ([`ldpk_model_registry.h`](ae_ldpk/ldpk_model_registry.h)), загрузчик ([`ldpk_nk_loader.h`](ae_ldpk/ldpk_nk_loader.h)), таблицы параметров ([`ldpk_param_tables.h`](ae_ldpk/ldpk_param_tables.h)), утилита сверки с Nuke ([`nk_ld_dump.cpp`](ae_ldpk/nk_ld_dump.cpp)), тест ([`test_snapshot_roundtrip.cpp`](ae_ldpk/test_snapshot_roundtrip.cpp)), исходник эффекта ([`ae/`](ae_ldpk/ae)) и проект VS 2022 ([`ae/Win/`](ae_ldpk/ae/Win)) |
| [`ldpk-2.14.0/`](ldpk-2.14.0) | Оригинальное дерево LDPK 2.14.0 (Science-D-Visions), без изменений: header-only модели ([`include/ldpk/`](ldpk-2.14.0/include/ldpk) — нужны для сборки моста), плагины Nuke/3DE ([`source/ldpk/`](ldpk-2.14.0/source/ldpk)), экспортёры 3DE4 ([`3de4/python/`](ldpk-2.14.0/3de4/python)), сборочные скрипты ([`script/`](ldpk-2.14.0/script)), тесты ([`test/`](ldpk-2.14.0/test)), VS-проекты Nuke ([`windows/`](ldpk-2.14.0/windows)), документация ([`doc/`](ldpk-2.14.0/doc)), лицензия ([`license`](ldpk-2.14.0/license)) |
| [`releases/`](releases) | Документация релизов; бинарники публикуются на странице Releases |

## Загрузка

Готовые плагины публикуются как ассеты **GitHub Releases**: <https://github.com/comkoff/ldpk-ae/releases>. В дереве репозитория — только исходники и документация. Каждый архив релиза содержит **обе сборки** (release и debug) и руководство пользователя на **русском и английском**.

## Сборка AE-плагина (Windows)

- Visual Studio 2022 (toolset v143, x64), workload «Desktop development with C++».
- After Effects SDK **не входит в репозиторий**: скачайте AE SDK 26.5 и распакуйте в `AfterEffectsSDK_26.5_win/` рядом с `ae_ldpk/`.
- Заголовки LDPK берутся из `ldpk-2.14.0/include` (прописано в [.vcxproj](ae_ldpk/ae/Win/LD3DE4_Lens_Distortion.vcxproj)).
- Подробные команды сборки — в [`ae_ldpk/readme.md`](ae_ldpk/readme.md): утилита `nk_ld_dump`, MSBuild для `LD3DE4_Lens_Distortion.sln`, диагностическая сборка с журналом `%TEMP%\ld3de4_ae.log`.

## Использование (кратко)

1. Эффект **Lens Distortion → 3DE4 Lens Distortion** на слое.
2. **Choose .nk File...** — файл из 3DE4 (подойдёт и полный `.nk` Nuke); модель и ключи подставятся автоматически, файл больше не нужен (снимок хранится в проекте).
3. **Direction**: Undistort / Distort.
4. **Fit Comp To Distortion...** — подгонка композиции под полный образ.

Полное руководство: [Русский](releases/AE_LD3DE4/readme_user_ru.md) | [English](releases/AE_LD3DE4/readme_user.md).

## Лицензия

Код моста — «as is», без гарантий. Модели LDPK — free software © Uwe Sassenberg, Science-D-Visions (см. [`ldpk-2.14.0/license`](ldpk-2.14.0/license)); заголовок `tde4_ld_plugin.h` принадлежит Science-D-Visions. Примеры Adobe SDK использованы как структурный референс.
