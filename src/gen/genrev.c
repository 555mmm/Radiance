/*

#ifndef lint
static char SCCSid[] = "$SunId$ LBL";
#endif
 *  genrev.c - program to generate functions of rotation about z
 *
 *	The program takes as input the functions of t for z and r
 *	(the radius).  If z is increasing, the normal points
 *	outward, inward for decreasing z.  Negative radii are forbidden.
 *
 *	8/6/86
 */

#include  <stdio.h>

#define  ZNAME		"Z_"			/* z function name */
#define  RNAME		"R_"			/* r function name */

#define  PI		3.14159265358979323846

#define  FTINY		1e-9
					/* orientation */
#define  OUT		01
#define  IN		02
#define  UP		04
#define  DOWN		010

double  funvalue(), l_hermite(), argument();


main(argc, argv)
int  argc;
char  *argv[];
{
	char  stmp[256];
	char  *modname;
	int  smooth = 0;
	double  t, lastz, z, nextz, lastr, r, nextr;
	double  lastnz, lastnr, nz, nr, nextnz, nextnr;
	int  i, nseg;
	int  orient;

	varset("PI", PI);
	funset("hermite", 5, l_hermite);

	if (argc < 6)
		goto userror;

	for (i = 6; i < argc; i++)
		if (!strcmp(argv[i], "-e"))
			scompile(NULL, argv[++i]);
		else if (!strcmp(argv[i], "-f"))
			fcompile(argv[++i]);
		else if (!strcmp(argv[i], "-s"))
			smooth = 1;
		else
			goto userror;

	sprintf(stmp, "%s(t)=%s;", ZNAME, argv[3]);
	scompile(NULL, stmp);
	sprintf(stmp, "%s(t)=%s;", RNAME, argv[4]);
	scompile(NULL, stmp);
	nseg = atoi(argv[5]);
	if (nseg <= 0)
		goto userror;
	modname = smooth ? "phong" : argv[1];

	printhead(argc, argv);

	lastnz = lastnr = 0.0;
	t = 0.0;
	lastz = funvalue(ZNAME, 1, &t);
	lastr = funvalue(RNAME, 1, &t);
	t = 1.0/nseg;
	z = funvalue(ZNAME, 1, &t);
	r = funvalue(RNAME, 1, &t);
	computen(&nz, &nr, lastz, lastr, z, r);
	for (i = 1; i <= nseg; i++) {
		if (i < nseg) {
			t = (double)(i+1)/nseg;
			nextz = funvalue(ZNAME, 1, &t);
			nextr = funvalue(RNAME, 1, &t);
			computen(&nextnz, &nextnr, z, r, nextz, nextr);
		} else
			nextnz = nextnr = 0.0;
		orient = 0;
		if (z < lastz-FTINY)
			orient |= DOWN;
		else if (z > lastz+FTINY)
			orient |= UP;
		if (r < lastr-FTINY)
			orient |= IN;
		else if (r > lastr+FTINY)
			orient |= OUT;
		if (!orient)
			goto endfor;
		if (smooth) {
			printf("\n%s texfunc phong\n", argv[1]);
			printf("4 rev_dx rev_dy rev_dz rev.cal\n");
			printf("0\n4\n");
			if (orient&(UP|DOWN)) {
				t = (nextnz - lastnz)/(z - lastz);
				printf("\t%15.9g\t%15.9g\n",
						t, lastnz - t*lastz);
			} else
				printf("\t0\t%d\n", orient&IN ? 1 : -1);
			if (orient&(OUT|IN))  {
				t = (nextnr - lastnr)/(r - lastr);
				printf("\t%15.9g\t%15.9g\n",
						t, lastnr - t*lastr);
			} else
				printf("\t0\t%d\n", orient&UP ? 1 : -1);
		}
		if (!(orient&(IN|OUT))) {
			printf("\n%s %s %s.%d\n", modname,
					orient&DOWN ? "tube" : "cylinder",
					argv[2], i);
			printf("0\n0\n7\n");
			printf("\t0\t0\t%15.9g\n", lastz);
			printf("\t0\t0\t%15.9g\n", z);
			printf("\t%15.9g\n", r);
		} else if (!(orient&(UP|DOWN))) {
			printf("\n%s ring %s.%d\n", modname, argv[2], i);
			printf("0\n0\n8\n");
			printf("\t0\t0\t%15.9g\n", z);
			printf("\t0\t0\t%15.9g\n", orient&IN ? 1.0 : -1.0);
			printf("\t%15.9g\t%15.9g\n", lastr, r);
		} else {
			printf("\n%s %s %s.%d\n", modname,
					orient&DOWN ? "cup" : "cone",
					argv[2], i);
			printf("0\n0\n8\n");
			printf("\t0\t0\t%15.9g\n", lastz);
			printf("\t0\t0\t%15.9g\n", z);
			printf("\t%15.9g\t%15.9g\n", lastr, r);
		}
	endfor:
		lastz = z; lastr = r;
		z = nextz; r = nextr;
		lastnz = nz; lastnr = nr;
		nz = nextnz; nr = nextnr;
	}
	quit(0);

userror:
	fprintf(stderr,
	"Usage: %s material name z(t) r(t) nseg [-e expr] [-f file] [-s]\n",
			argv[0]);
	quit(1);
}


computen(nzp, nrp, z0, r0, z1, r1)		/* compute normal */
double  *nzp, *nrp, z0, r0, z1, r1;
{
	extern double  sqrt();
	double  dr, dz, len;

	dz = r0 - r1;				/* right angle vector */
	dr = z1 - z0;
	len = sqrt(dr*dr + dz*dz);
	*nzp = dz/len;
	*nrp = dr/len;
}


eputs(msg)
char  *msg;
{
	fputs(msg, stderr);
}


wputs(msg)
char  *msg;
{
	eputs(msg);
}


quit(code)
{
	exit(code);
}


printhead(ac, av)		/* print command header */
register int  ac;
register char  **av;
{
	putchar('#');
	while (ac--) {
		putchar(' ');
		fputs(*av++, stdout);
	}
	putchar('\n');
}


double
l_hermite()			
{
	double  t;
	
	t = argument(5);
	return(	argument(1)*((2.0*t-3.0)*t*t+1.0) +
		argument(2)*(-2.0*t+3.0)*t*t +
		argument(3)*((t-2.0)*t+1.0)*t +
		argument(4)*(t-1.0)*t*t );
}
