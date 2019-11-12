#include <fs/fs.h>

char* z_fgets(char* s, int n, struct fs_file_t *file)
{
    int c;
    int res;
    char* cs;
    cs = s;

    while (--n > 0 && (res = fs_read(file, &c, 1) == 1))
    {
    // put the input char into the current pointer position, then increment it
    // if a newline entered, break
    if ((*cs++ = c) == '\n')
        break;
    }

    *cs = '\0';
    return (res <= 0 && cs == s) ? NULL : s;
}