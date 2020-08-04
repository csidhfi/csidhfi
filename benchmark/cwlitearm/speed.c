#include "hal.h"
#include "csidh.h"
#include "sendfn.h"
#include "systick.h"
#include "mont.h"

#include <string.h>
#include <stdio.h>

#define printcycles(S, U) send_unsignedll((S), (U))

void hal_send_str(const char *s)
{
    while (*s)
    {
        putch(*(s++));
    }
    putch('\n');
}

const unsigned primes[num_primes] = {359, 353, 349, 347, 337, 331, 317, 313, 311,
                                     307, 293, 283, 281, 277, 271, 269, 263, 257, 251, 241, 239, 233, 229,
                                     227, 223, 211, 199, 197, 193, 191, 181, 179, 173, 167, 163, 157, 151,
                                     149, 139, 137, 131, 127, 113, 109, 107, 103, 101, 97, 89, 83, 79, 73,
                                     71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 5, 3,
                                     587, 373, 367};

static void benchmark_isogs(void) {
   proj A, P, Pd, K;
   unsigned long long t0, t1;
   char str[100];
   for(int i=0;i<num_primes;i++){
      t0 = hal_get_time();
      xISOG(&A, &P, &Pd, &K, primes[i], 0);
      t1 = hal_get_time();

      sprintf(str, "xISOG(k=%d) cycles:", primes[i]);
      printcycles(str, t1 - t0);
   }
}

int main(void)
{
  platform_init();
  init_uart();
  systick_setup();

  unsigned long long t0, t1;

  hal_send_str("==========================");
  benchmark_isogs();
  private_key priv_bob, priv_alice;
  public_key pub_bob, pub_alice;
  public_key shared_alice, shared_bob;
  int8_t max[num_primes] = {2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
                            5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8,
                            9, 9, 9, 10, 10, 10, 10, 9, 8, 8, 8, 7, 7, 7, 7, 7, 6, 5,
                            1, 2, 2};
  uint8_t  num_batches   = 3;
  uint8_t my = 8;
  unsigned int num_isogenies = 404;

  t0 = hal_get_time();
  csidh_private(&priv_alice , max);
  t1 = hal_get_time();
  printcycles("private alice cycles:", t1-t0);

  t0 = hal_get_time();
  csidh_private(&priv_bob , max);
  t1 = hal_get_time();
  printcycles("private bob cycles:", t1-t0);

  t0 = hal_get_time();
  csidh(&pub_alice, &base, &priv_alice, num_batches, max, num_isogenies, my);
  t1 = hal_get_time();
  printcycles("public alice cycles:", t1-t0);

  t0 = hal_get_time();
  csidh(&pub_bob, &base, &priv_bob, num_batches, max, num_isogenies, my);
  t1 = hal_get_time();
  printcycles("public bob cycles:", t1-t0);

  t0 = hal_get_time();
  csidh(&shared_alice, &pub_bob, &priv_alice, num_batches, max, num_isogenies, my);
  t1 = hal_get_time();
  printcycles("shared alice cycles:", t1-t0);

  t0 = hal_get_time();
  csidh(&shared_bob, &pub_alice, &priv_bob, num_batches, max, num_isogenies, my);
  t1 = hal_get_time();
  printcycles("shared bob cycles:", t1-t0);

  if(memcmp(&shared_alice, &shared_bob, sizeof(public_key))){
     hal_send_str("ERROR");
  } else {
     hal_send_str("OK KEYS");
  }


  hal_send_str("#");
}

