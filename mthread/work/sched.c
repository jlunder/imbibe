#include <stdio.h>
#include <conio.h>

int main(void)
{
  int o[10];
  int p[10] = {1, 5, 3, 2, 3, 8, 20, 1, 1, 2};
  int s[10];
  int i;
  int total = 0;
  int executed = 0;

  for(i = 0; i < 10; ++i) total += p[i];
  for(i = 0; i < 10; ++i) o[i] = total;
  for(i = 0; i < 10; ++i) s[i] = p[i];

  printf("total: %d\n", total);

  do
  {
    executed = 0;
    while(!executed)
    {
      for(i = 0; i < 10; ++i)
      {
        o[i] -= p[i];
      }
      putc('.', stdout);
      for(i = 0; i < 10; ++i)
      {
        if(o[i] <= 0)
        {
          o[i] += total;
          s[i]++;
          executed = 1;
          printf("%d", i);
          break;
        }
      }
      flushall();
    }
  } while(!kbhit());
  while(kbhit()) getch();

  printf("\n");
  for(i = 0; i < 10; ++i) printf("%d ", s[i]);
  printf("\n");
  for(i = 0; i < 10; ++i) printf("%d ", p[i]);
  printf("\n");
  return 0;
}
