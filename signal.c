#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static sigset_t sigset_mask;

void signal_block(int sig)
{
  sigaddset(&sigset_mask, sig);
  sigprocmask(SIG_BLOCK, &sigset_mask, NULL);
}

void signal_unblock(int sig)
{
  static sigset_t tmp_mask;
  
  sigemptyset(&tmp_mask);
  sigaddset(&tmp_mask, sig);
  sigprocmask(SIG_UNBLOCK, &tmp_mask, NULL);
  sigdelset(&sigset_mask, sig);
}


void signal_action(int sig, void (*handler)(int))
{
  struct sigaction sact;
  
  memset(&sact, 0, sizeof(sact));
  sact.sa_handler = handler;
  sact.sa_flags = SA_RESTART;
  sigaction(sig, &sact, NULL);
}
