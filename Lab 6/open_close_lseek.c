int open_file()
{
  int mode;
  printf("[ ENTERED KOPEN() FUNCTION ]\n");
  printf("0 = R\n1 = W\n2 = RW\n3 = APPEND\nInput a Value:\n>> ");

  char pathname[128];
  /// 1. ask for a pathname and mode to open:
  mode = geti();
  switch(mode) {
    case 1:
    printf("[ MODE: R ]\n");

    break;
    case 2:
    printf("[ MODE: W ]\n");

    break;
    case 3:
    printf("[ MODE: RW ]\n");

    break;
    case 4:
    printf("[ MODE: APPEND ]\n");

    break;
  }

  /// 2. get pathname's inumber:

  /// 3. get its Minode pointer

  /// 4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.

  /// 5. allocate a FREE OpenFileTable (OFT) and fill in values:

  /// 6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

  /// 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL

  /// 8. update INODE's time field

  /// 9. return i as the file descriptor

  return 0;
}

void tokenize() {

}

/*int truncate(MINODE *mip)
{
  return 0;
}*/

int close_file(int fd)
{
  return 0;
}