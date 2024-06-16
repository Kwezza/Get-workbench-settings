#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/iffparse.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <stdio.h>
#include <prefs/workbench.h> 

/*
 * This program reads the Workbench preferences file from the Amiga's 
 * "ENV:sys" directory and extracts various Workbench settings into a 
 * user-defined structure. The settings include borderless mode, emboss 
 * rectangle size, maximum name length, new icons support, color icon support, 
 * and optional extended settings like disabling the title bar and volume gauge.
 * 
 * The program opens the Workbench preferences file, parses the IFF (Interchange 
 * File Format) content to locate the 'WBNC' (Workbench New Configuration) chunk, 
 * and reads the relevant settings. It then prints these settings to the console.
 * 
 * This utility simplifies access to Workbench settings, avoiding the need to 
 * understand the underlying format of the settings stored on the Amiga.
 */

/* Path to Workbench preferences file */
#define PREFS_FILE "ENV:sys/Workbench.prefs"

/* Define the WorkbenchSettings structure */
struct WorkbenchSettings {
    BOOL borderless;
    LONG embossRectangleSize;
    LONG maxNameLength;
    BOOL newIconsSupport;
    BOOL colorIconSupport;
    BOOL disableTitleBar;
    BOOL disableVolumeGauge;
};

/* Function to read and extract Workbench settings */
BOOL ReadWorkbenchSettings(BPTR file, struct WorkbenchSettings *settings) {
    BOOL result = FALSE;
    UBYTE buffer[12];
    ULONG chunkType, chunkSize;
    struct WorkbenchPrefs *prefs = NULL;
    struct WorkbenchExtendedPrefs *extPrefs = NULL;
    UBYTE *chunkData;

    /* Read the FORM header (12 bytes: 'FORM', size, 'PREF') */
    if (Read(file, buffer, 12) == 12) {
        chunkType = *((ULONG *)(buffer + 8)); /* Chunk type should be 'PREF' */
        if (chunkType != MAKE_ID('P', 'R', 'E', 'F')) {
            return FALSE;
        }

        while (Read(file, buffer, 8) == 8) {
            /* Read chunk header (8 bytes: ID, size) */
            chunkType = *((ULONG *)buffer);
            chunkSize = *((ULONG *)(buffer + 4));
            chunkSize = (chunkSize + 1) & ~1; /* Ensure even size for padding */

            /* Allocate buffer to read the chunk */
            chunkData = (UBYTE *)AllocMem(chunkSize, MEMF_PUBLIC | MEMF_CLEAR);
            if (chunkData) {
                if (Read(file, chunkData, chunkSize) == chunkSize) {
                    /* Check if this is the WBNC chunk */
                    if (chunkType == MAKE_ID('W', 'B', 'N', 'C')) {
                        prefs = (struct WorkbenchPrefs *)chunkData;

                        settings->borderless = prefs->wbp_Borderless;
                        settings->embossRectangleSize = prefs->wbp_EmbossRect.MaxX;
                        settings->maxNameLength = prefs->wbp_MaxNameLength;
                        settings->newIconsSupport = prefs->wbp_NewIconsSupport;
                        settings->colorIconSupport = prefs->wbp_ColorIconSupport;

                        /* Check if the chunk size is large enough to include extended preferences */
                        if (chunkSize > sizeof(struct WorkbenchPrefs)) {
                            extPrefs = (struct WorkbenchExtendedPrefs *)chunkData;

                            settings->disableTitleBar = extPrefs->wbe_DisableTitleBar;
                            settings->disableVolumeGauge = extPrefs->wbe_DisableVolumeGauge;
                        }

                        result = TRUE;
                        break;
                    }
                }
                FreeMem(chunkData, chunkSize);
            }
        }
    }

    return result;
}

/* Example usage */
int main() {
    struct WorkbenchSettings settings;
    BPTR file;

    printf("Opening Workbench preferences file: %s\n", PREFS_FILE);

    file = Open(PREFS_FILE, MODE_OLDFILE);
    if (file) {
        if (ReadWorkbenchSettings(file, &settings)) {
            printf("Borderless: %s\n", settings.borderless ? "Yes" : "No");
            printf("Emboss Rectangle Size: %ld\n", settings.embossRectangleSize);
            printf("Max Name Length: %ld\n", settings.maxNameLength);
            printf("New Icons Support: %s\n", settings.newIconsSupport ? "Yes" : "No");
            printf("Color Icon Support: %s\n", settings.colorIconSupport ? "Yes" : "No");
            printf("Disable Title Bar: %s\n", settings.disableTitleBar ? "Yes" : "No");
            printf("Disable Volume Gauge: %s\n", settings.disableVolumeGauge ? "Yes" : "No");
        } else {
            printf("Failed to read Workbench settings.\n");
        }
        Close(file);
    } else {
        printf("Failed to open file: %s\n", PREFS_FILE);
    }

    return 0;
}
