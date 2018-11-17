#include <stdio.h>

#ifdef MAX
#undef MAX
#endif

#define NUM_MOB 25600

#define MAX(a, b) (((a)>(b))?(a):(b))

int main(int argc, char *argv[])
{
  int i, zone, mob, limit, garbage, total, num_multiple, num;
  int temp1, temp2, temp3, temp4;
  float multi, newlim;
  FILE *fp, *fl;
  char buf[256];
  struct {
    int limit;
    int number;
  } moblist[NUM_MOB];

  for(i=1; i<argc; i++) {
    for(mob=0; mob<NUM_MOB; mob++) {
      moblist[mob].limit=0;
      moblist[mob].number=0;
    }
    if(!(fl=fopen(argv[i], "r"))) {
      printf("Error opening %s\n", argv[i]);
      continue;
    }
    if(!(fp=fopen("tempzone.zon", "w"))) {
      printf("Error creating temp file\n");
      fclose(fl);
      exit(1);
    }
    fgets(buf, 256, fl);
    sscanf(buf, "#%d", &zone);
    fgets(buf, 256, fl);
    fgets(buf, 256, fl);
    while(!feof(fl)) {
      fgets(buf, 256, fl);
      if((!feof(fl)) && (*buf=='M')) {
        sscanf(buf+1, " %d %d %d %d %d %d ", &garbage, &garbage, &mob, &limit, &garbage, &garbage);
        if((mob < 0) || (mob > NUM_MOB-1)) {
          printf("Error, mob outside of range: %s", buf);
          fclose(fl);
          fclose(fp);
          exit(1);
        }
        newlim=0.49 + ((float)(moblist[mob].number*moblist[mob].limit)+limit)/(moblist[mob].number+1.0);
        moblist[mob].limit=(int)newlim;
        moblist[mob].number++;
      }
    }
    rewind(fl);
    total=0;
    num_multiple=0;
    for(mob=0; mob<NUM_MOB; mob++) {
      if(moblist[mob].number) {
        moblist[mob].limit/=moblist[mob].number;
        moblist[mob].limit = MAX(moblist[mob].limit, 1);
        total++;
        if(moblist[mob].limit > 1)
          num_multiple++;
      }
    }
    if(num_multiple && (total/num_multiple >= 2)) {
      total=num=0;
      for(mob=0; mob<NUM_MOB; mob++) {
        num += moblist[mob].number;
        total += moblist[mob].number*moblist[mob].limit;
      }
      if(total/num > 4)
        multi=0.5;
      else if(total/num > 2)
        multi=2.0/3.0;
      else
        multi=1.0;
      for(mob=0; mob<NUM_MOB; mob++) {
        newlim=moblist[mob].limit*multi;
        moblist[mob].limit=(int)newlim;
        if(newlim-moblist[mob].limit >= 0.5)
          moblist[mob].limit++;
        moblist[mob].limit = MAX(moblist[mob].limit, 1);
      }
    }
    for(mob=0; mob<NUM_MOB; mob++) {
      if(moblist[mob].limit > 60)
        moblist[mob].limit = 30;
      else if(moblist[mob].limit > 30)
        moblist[mob].limit >>= 1;
      else if(moblist[mob].limit > 15)
        moblist[mob].limit = 15;
    }
    fgets(buf, 256, fl);
    fputs(buf, fp);
    fgets(buf, 256, fl);
    fputs(buf, fp);
    fgets(buf, 256, fl);
    fputs(buf, fp);
    do {
      fgets(buf, 256, fl);
      if((!feof(fl)) && (*buf=='M')) {
        sscanf(buf+1, " %d %d %d %d %d %d ", &temp1, &temp2, &mob, &garbage, &temp3, &temp4);
        sprintf(buf, "M %d %d %d %d %d %d\r\n", temp1, temp2, mob, moblist[mob].limit, temp3, temp4);
      }
      if(!feof(fl))
        fputs(buf, fp);
    } while(!feof(fl));
    fclose(fl);
    fclose(fp);
    sprintf(buf, "new/%s", argv[i]);
    rename("tempzone.zon", buf);
  }
  exit(0);
}
