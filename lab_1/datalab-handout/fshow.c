/* Display structure of floating-point numbers */

#include <stdio.h>
#include <stdlib.h>
float strtof(const char *nptr, char **endptr);

#define FLOAT_SIZE 32
#define FRAC_SIZE 23
#define EXP_SIZE 8
#define BIAS ((1<<(EXP_SIZE-1))-1)
#define FRAC_MASK ((1<<FRAC_SIZE)-1)
#define EXP_MASK ((1<<EXP_SIZE)-1)

/* Floating point helpers */
unsigned f2u(float f)
{
  union {
    unsigned u;
    float f;
  } v;
  v.u = 0;
  v.f = f;
  return v.u;
}

static float u2f(unsigned u)
{
  union {
    unsigned u;
    float f;
  } v;
  v.u = u;
  return v.f;
}

/* Get exponent */
unsigned get_exp(unsigned uf)
{
  // << means move every bit 1 position left
  // after right shift => 00..00 sign-bit [exp 8 bits]
  // 1 => 0000 0001 << 8 => 1 0000 0000 - 0000 0001 => 0 1111 1111
  // 00..00 sign-bit [exp 8 bits] & 0 1111 1111 => keep only the 8 most right bits [bits of exp]
  return (uf>>FRAC_SIZE) & EXP_MASK;
}

/* Get fraction */
unsigned get_frac(unsigned uf)
{
  // follow the same steps used above in exp, here
  return uf & FRAC_MASK;
}

/* Get sign */
unsigned get_sign(unsigned uf)
{
  return (uf>>(FLOAT_SIZE-1)) & 0x1; // keep bit with index 31
}

void show_float(unsigned uf)
{
  float f = u2f(uf);
  unsigned exp = get_exp(uf);
  unsigned frac = get_frac(uf);
  unsigned sign = get_sign(uf);

  printf("\nFloating point value %.10g\n", f);
  printf("Bit Representation 0x%.8x, sign = %x, exponent = 0x%.2x, fraction = 0x%.6x\n",
	 uf, sign, exp, frac);
  if (exp == EXP_MASK) {
    if (frac == 0) {
      printf("%cInfinity\n", sign ? '-' : '+');
    } else
      printf("Not-A-Number\n");
  } else {
    int denorm = (exp == 0);
    int uexp = denorm ? 1-BIAS : exp - BIAS;
    int mantissa = denorm ? frac : frac + (1<<FRAC_SIZE);
    float fman = (float) mantissa / (float) (1<<FRAC_SIZE);
    printf("%s.  %c%.10f X 2^(%d)\n",
	   denorm ? "Denormalized" : "Normalized",
	   sign ? '-' : '+',
	   fman, uexp);
  }
}

/* Extract hex/decimal/or float value from string */
static int get_num_val(char *sval, unsigned *valp) {
  char *endp;
  /* See if it's an integer or floating point */
  int ishex = 0;
  int isfloat = 0;
  int i;
  for (i = 0; sval[i]; i++) {
    switch (sval[i]) {
    case 'x':
    case 'X':
      ishex = 1;
      break;
    case 'e':
    case 'E':
      if (!ishex)
	isfloat = 1;
      break;
    case '.':
      isfloat = 1;
      break;
    default:
      break;
    }
  }
  if (isfloat) {
    float fval = strtof(sval, &endp);
    if (!*endp) {
      *valp = *(unsigned *) &fval;
      return 1;
    }
    return 0;
  } else {
    long long int llval = strtoll(sval, &endp, 0);
    long long int upperbits = llval >> 31;
    /* will give -1 for negative, 0 or 1 for positive */
    if (valp && (upperbits == 0 || upperbits == -1 || upperbits == 1)) {
      *valp = (unsigned) llval;
      return 1;
    }
    return 0;
  }
}


void usage(char *fname) {
  printf("Usage: %s val1 val2 ...\n", fname);
  printf("Values may be given as hex patterns or as floating point numbers\n");
  exit(0);
}


int main(int argc, char *argv[])
{
  int i;
  unsigned uf;
  if (argc < 2)
    usage(argv[0]);
  for (i = 1; i < argc; i++) {
    char *sval = argv[i];
    if (get_num_val(sval, &uf)) {
      show_float(uf);
    } else {
      printf("Invalid 32-bit number: '%s'\n", sval);
      usage(argv[0]);
    }
  }
  return 0;
}


