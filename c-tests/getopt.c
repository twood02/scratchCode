#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
parse_1(int argc, char *argv[])
{
        int aflag = 0;
        int bflag = 0;
        char *cvalue = NULL;
        int index;
        int c;

        opterr = 0;

        while ((c = getopt (argc, argv, "abc:")) != -1)
          switch (c)
            {
            case 'a':
              aflag = 1;
              break;
            case 'b':
              bflag = 1;
              break;
            case 'c':
              cvalue = optarg;
              break;
            case '?':
              if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
              else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
              else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
              return 1;
            default:
              abort ();
            }

        printf ("aflag = %d, bflag = %d, cvalue = %s, inded = %d\n",
                aflag, bflag, cvalue, optind);
}

int
parse_2(int argc, char *argv[])
{
        int dflag = 0;
        int eflag = 0;
        char *cvalue = NULL;
        int index;
        int c;

        opterr = 0;

        while ((c = getopt (argc, argv, "c:de")) != -1)
          switch (c)
            {
            case 'd':
              dflag = 1;
              break;
            case 'e':
              eflag = 1;
              break;
            case 'c':
              cvalue = optarg;
              break;
            case '?':
              if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
              else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
              else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
              return 1;
            default:
              abort ();
            }

        printf ("dflag = %d, eflag = %d, cvalue = %s, index = %d\n",
                dflag, eflag, cvalue, optind);

        return optind;
}

int
parse_3(int argc, char *argv[])
{
        int dflag = 0;
        int eflag = 0;
        char *cvalue = NULL;
        int index;
        int c;

        opterr = 0;

        while ((c = getopt (argc, argv, "d:fg")) != -1)
          switch (c)
            {
            case 'f':
              dflag = 1;
              break;
            case 'g':
              eflag = 1;
              break;
            case 'd':
              cvalue = optarg;
              break;
            case '?':
              if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
              else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
              else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
              return 1;
            default:
              abort ();
            }

        printf ("fflag = %d, gflag = %d, dvalue = %s, index = %d\n",
                dflag, eflag, cvalue, optind);

        return optind;
}
/*
 This can handle multiple sets of arguments, as long as they are separated
 by a double dash (--)

 $ ./a.out -a -c "parse 1" -- -c "parse 2" -d -- -d "parse 3" -g
 aflag = 1, bflag = 0, cvalue = parse 1, inded = 5
 dflag = 1, eflag = 0, cvalue = parse 2, index = 9
 fflag = 0, gflag = 1, dvalue = parse 3, index = 12

 */


int
main (int argc, char *argv[])
{
        parse_1(argc, argv);
        parse_2(argc, argv);
        parse_3(argc, argv);
        return 0;
}
