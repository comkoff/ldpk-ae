/*******************************************************************/
/*                                                                 */
/* LD_3DE4 Lens Distortion for After Effects — macOS file dialog   */
/*                                                                 */
/* NSOpenPanel-based replacement for the Win32 GetOpenFileNameA    */
/* call in LD_PickNkFile() (LD3DE4_Lens_Distortion.cpp). Compiled  */
/* only on macOS (Objective-C++); the Windows build never sees     */
/* this file.                                                      */
/*                                                                 */
/* Software is provided "as is" - no warranties implied.           */
/*                                                                 */
/*******************************************************************/

#import <AppKit/AppKit.h>

#include <cstring>

/* Returns true and fills path (NUL-terminated) when the user picks a
   .nk file; returns false on cancel. On entry path may hold the
   previously imported file (the caller pre-fills it) — the panel starts
   in that folder. */

bool LD_MacPickNkFile(char* path, size_t path_size)
{
    @autoreleasepool
    {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations" // allowedFileTypes: fine before macOS 12 API
        [panel setAllowedFileTypes:@[@"nk"]];
#pragma clang diagnostic pop

        // Start in the folder of the previously imported file (if any).
        if (path[0])
        {
            NSString* prev = [NSString stringWithUTF8String:path];
            NSString* dir = [prev stringByDeletingLastPathComponent];
            if ([dir length] > 0)
            {
                [panel setDirectoryURL:[NSURL fileURLWithPath:dir]];
            }
        }

        if ([panel runModal] != NSModalResponseOK)
        {
            return false;
        }

        NSString* picked = [[panel URLs] firstObject].path;
        if (!picked)
        {
            return false;
        }

        strncpy(path, picked.fileSystemRepresentation, path_size - 1);
        path[path_size - 1] = 0;
        return true;
    }
}
