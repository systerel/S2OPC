#!/bin/env gawk
##########################################################################
#                                                                        #
#  This file is part of deliverable T3.3 of project INGOPCS              #
#                                                                        #
#    Copyright (C) 2017-2018 TrustInSoft                                 #
#                                                                        #
#  All rights reserved.                                                  #
#                                                                        #
##########################################################################

BEGIN {
  print "#include <stdint.h>"
  print "#include <sys/types.h>"
  print "#include <string.h>"
}
/tis-input/ {
  split($3, a, ":")
  fct = a[1];
  name = a[2];
  call = a[3];
  value = $5;
  tbl[fct][name][call] = value;
}

/^uint8_t msg/ { nb_msg++; msg = 1; }
msg == 1 { print $0; }
msg == 1 && /}/ { msg = 0; }

/^uint8_t pData_/ { nb_pData++; pData = 1; }
pData == 1 { print $0; }
pData == 1 && /}/ { pData = 0; }

END {
  print \
    "int tis_force_value (const char * f, const char * id, size_t n, int old) {";
  for (fct in tbl) {
    print "  if (0 == strcmp (f, \"" fct "\")) {"
    for (name in tbl[fct]) {
      print "    if (0 == strcmp (id, \"" name "\")) {";
      print "      switch (n) {";
      for (call in tbl[fct][name]) {
        value=tbl[fct][name][call];
        print  "        case " call ":";
        printf "          //@ assert force_%s_%s_%d: old == %d;\n", fct, name, call, value;
        print  "          break;";
          }
          printf "        default: //@ assert force_%s_%s_missing_value: \\false;\n",
                 fct, name;
          print "          break;";
          print "      }"
          print "      return old;"
          print "    }"
      }
      printf "    //@ assert force_%s_missing_variable: \\false;\n", fct;
      print "  }"
    }
    printf "  //@ assert force_missing_function: \\false;\n";
    print "  return 0;";
    print "}"
    print "";
    print "ssize_t recv(int sockfd, void *buf, size_t len, int flags) {";
    print "  static size_t nb_call = 1; size_t msglen;";
    print "  switch (nb_call++) {"
    for (i = 1; i <= nb_msg; i++) {
      printf "    case %d:", i;
      printf " msglen = sizeof (msg_%d); memcpy (buf, msg_%d, msglen);", i, i;
      print " break;"
      }
    print "    default: ; //@ assert missing_recv: \\false;";
    print "  }";
    print "  return msglen;"
    print "}"
    print "";
    print "void tis_force_pData (uint8_t * pData, uint32_t lenData, ";
    print "                      size_t call) {";
    print "  size_t msglen;"
    print "  switch (call) {"
    for (i = 1; i <= nb_pData; i++) {
      printf "    case %d:\n", i;
      printf "      msglen = sizeof (pData_%d);\n", i;
      printf "      //@ assert pData_%d_len: msglen == lenData;\n", i;
      printf "      memcpy (pData, pData_%d, msglen);\n", i;
      print  "      break;"
      }
    print "    default: ; //@ assert missing_pData: \\false;";
    print "  }";
    print "}";
  }
