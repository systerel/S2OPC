
// dummy main to call the real 'main' with no argument.
int tis_main (void) {
  int main(int argc, char *argv[]);
  char * argv [] = { "dummy", 0 };
  return main (1, argv);
}

