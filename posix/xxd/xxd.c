/* xxd — hex dump / reverse hex dump utility
 *
 * Usage: xxd [options] [file]
 *   -l len     stop after len bytes
 *   -s offset  skip offset bytes from start
 *   -c cols    columns per line (default 16)
 *   -p         plain hex dump (no addresses or ASCII)
 *   -r         reverse: convert hex dump back to binary
 *   -i         C include style output
 *
 * Reads from stdin if no file given.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void hex_dump(FILE *fp, long offset, long length, int cols, int plain, int c_include, const char *name)
{
    unsigned char buf[256];
    long addr = 0;
    long total = 0;
    int n, i;

    if (offset > 0)
        fseek(fp, offset, SEEK_SET);

    if (c_include) {
        /* sanitize name for C identifier */
        printf("unsigned char");
        if (name) {
            printf(" %s", name);
            /* replace non-alnum with _ */
        }
        printf("[] = {\n");
    }

    while ((n = fread(buf, 1, (cols < (int)sizeof(buf)) ? cols : (int)sizeof(buf), fp)) > 0) {
        if (length >= 0 && total + n > length)
            n = (int)(length - total);
        if (n <= 0)
            break;

        if (c_include) {
            printf(" ");
            for (i = 0; i < n; i++) {
                printf(" 0x%02x", buf[i]);
                if (total + i + 1 < length || length < 0)
                    printf(",");
            }
            printf("\n");
        } else if (plain) {
            for (i = 0; i < n; i++)
                printf("%02x", buf[i]);
            printf("\n");
        } else {
            /* address */
            printf("%08lx: ", addr + offset);

            /* hex bytes */
            for (i = 0; i < cols; i++) {
                if (i < n)
                    printf("%02x", buf[i]);
                else
                    printf("  ");
                if (i % 2 == 1)
                    printf(" ");
            }

            /* ASCII */
            printf(" ");
            for (i = 0; i < n; i++) {
                if (buf[i] >= 0x20 && buf[i] <= 0x7e)
                    putchar(buf[i]);
                else
                    putchar('.');
            }
            printf("\n");
        }

        addr += n;
        total += n;
        if (length >= 0 && total >= length)
            break;
    }

    if (c_include)
        printf("};\nunsigned int %s_len = %ld;\n", name ? name : "data", total);
}

static int hex_val(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void reverse_dump(FILE *fp)
{
    int c, hi, lo;

    while ((c = fgetc(fp)) != EOF) {
        /* skip non-hex: whitespace, ':', addresses, ASCII column */
        hi = hex_val(c);
        if (hi < 0)
            continue;
        lo = fgetc(fp);
        if (lo == EOF)
            break;
        lo = hex_val(lo);
        if (lo < 0)
            continue;
        putchar((hi << 4) | lo);
    }
}

int main(int argc, char **argv)
{
    long offset = 0;
    long length = -1;
    int cols = 16;
    int plain = 0;
    int reverse = 0;
    int c_include = 0;
    const char *filename = NULL;
    FILE *fp;
    int i;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            switch (argv[i][1]) {
            case 'l':
                if (argv[i][2])
                    length = strtol(&argv[i][2], NULL, 0);
                else if (++i < argc)
                    length = strtol(argv[i], NULL, 0);
                break;
            case 's':
                if (argv[i][2])
                    offset = strtol(&argv[i][2], NULL, 0);
                else if (++i < argc)
                    offset = strtol(argv[i], NULL, 0);
                break;
            case 'c':
                if (argv[i][2])
                    cols = (int)strtol(&argv[i][2], NULL, 0);
                else if (++i < argc)
                    cols = (int)strtol(argv[i], NULL, 0);
                if (cols < 1) cols = 16;
                if (cols > 256) cols = 256;
                break;
            case 'p':
                plain = 1;
                break;
            case 'r':
                reverse = 1;
                break;
            case 'i':
                c_include = 1;
                break;
            case 'h':
                fprintf(stderr, "Usage: xxd [-l len] [-s offset] [-c cols] [-p] [-r] [-i] [file]\n");
                return 0;
            default:
                fprintf(stderr, "xxd: unknown option '-%c'\n", argv[i][1]);
                return 1;
            }
        } else {
            filename = argv[i];
        }
    }

    if (filename) {
        fp = fopen(filename, "rb");
        if (!fp) {
            fprintf(stderr, "xxd: cannot open '%s'\n", filename);
            return 1;
        }
    } else {
        fp = stdin;
    }

    if (reverse)
        reverse_dump(fp);
    else
        hex_dump(fp, offset, length, cols, plain, c_include, filename);

    if (fp != stdin)
        fclose(fp);

    return 0;
}
