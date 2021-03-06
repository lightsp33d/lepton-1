/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Chauvin-Arnoux.
Portions created by Chauvin-Arnoux are Copyright (C) 2011. All Rights Reserved.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/

/*============================================
| Includes
==============================================*/
#include "kernel/core/errno.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/syscall.h"
#include "kernel/core/kernel_pthread.h"
#include "kernel/core/kernel_sem.h"

#include "kernel/core/time.h"
#include "kernel/core/kernel_clock.h"


/*============================================
| Global Declaration
==============================================*/


/*============================================
| Implementation
==============================================*/

/*--------------------------------------------
| Name:        kernel_sem_init
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_init(kernel_sem_t* kernel_sem, int pshared, unsigned int value){
   if(!kernel_sem)
      return -1;
#if defined(__KERNEL_UCORE_ECOS)
   cyg_semaphore_init(&kernel_sem->sem, (char)value);
#endif

   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_destroy
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_destroy(kernel_sem_t* kernel_sem){
   if(!kernel_sem)
      return -1;
#if defined(__KERNEL_UCORE_ECOS)
   cyg_semaphore_destroy(&kernel_sem->sem);
#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_getvalue
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_getvalue(kernel_sem_t* kernel_sem, int *value){
   if(!kernel_sem)
      return -1;
#if defined(__KERNEL_UCORE_ECOS)
   cyg_semaphore_peek(&kernel_sem->sem, (cyg_count32 *)value);
#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_post
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_post(kernel_sem_t* kernel_sem){
   if(!kernel_sem)
      return -1;
#if defined(__KERNEL_UCORE_ECOS)
   cyg_semaphore_post(&kernel_sem->sem);
#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_timedwait
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_timedwait(kernel_sem_t* kernel_sem, int flag, const struct timespec * abs_timeout){
   int timeout=0;

   //
   if(!kernel_sem)
      return -1;
   //
   if(flag==TIMER_ABSTIME && abs_timeout) { //warning: on 16bit architecture use ldiv instead '/' for division with long type.
      //OS_DI();
      timeout = kernel_clock_timeout(CLOCK_REALTIME,abs_timeout);
   } if(!flag && abs_timeout) {
      timeout = __time_s_to_ms(abs_timeout->tv_sec)+__time_ns_to_ms(abs_timeout->tv_nsec);
   }
   //
#if defined(__KERNEL_UCORE_ECOS)
   if(abs_timeout && timeout) {
      if(!cyg_semaphore_timed_wait(&kernel_sem->sem, __kernel_get_timer_ticks()+timeout)) {
         __kernel_set_errno(-EBUSY);
         return -1;
      }
   }
   else if(abs_timeout && !timeout) {
      if(!cyg_semaphore_timed_wait(&kernel_sem->sem, 0)) {
         __kernel_set_errno(-EBUSY);
         return -1;
      }
   }
   else{
      if(!cyg_semaphore_wait(&kernel_sem->sem)) {
         __kernel_set_errno(-EINTR);
         return -1;
      }
   }

#endif
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_trywait*
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_trywait(kernel_sem_t* kernel_sem){
   if(!kernel_sem)
      return -1;
#if defined(__KERNEL_UCORE_ECOS)
   if(!cyg_semaphore_trywait(&kernel_sem->sem)) {
      __kernel_set_errno(-EBUSY);
      return -1;
   }
#endif

   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_wait
| Description:
| Parameters:  none
| Return Type: none
| Comments:
| See:
----------------------------------------------*/
int kernel_sem_wait(kernel_sem_t* kernel_sem){
   if(!kernel_sem)
      return -1;
#if defined(__KERNEL_UCORE_ECOS)
   if(!cyg_semaphore_wait(&kernel_sem->sem)) {
      __kernel_set_errno(-EINTR);
      return -1;
   }
#endif
   return 0;
}

//
#if 0
// the following function is not used at kernel level

/*--------------------------------------------
| Name:        kernel_sem_open
| Description:
| Parameters:  none
| Return Type: none
| Comments: named sem not used at kernel level
| See:
----------------------------------------------*/
kernel_sem_t * kernel_sem_open(const char * name, , ...){
   return (kernel_sem_t * )0;
}

/*--------------------------------------------
| Name:        kernel_sem_close
| Description:
| Parameters:  none
| Return Type: none
| Comments: named sem not used at kernel level
| See:
----------------------------------------------*/
int kernel_sem_close(kernel_sem_t* kernel_sem){
   return 0;
}

/*--------------------------------------------
| Name:        kernel_sem_unlink
| Description:
| Parameters:  none
| Return Type: none
| Comments: named sem not used at kernel level
| See:
----------------------------------------------*/
int kernel_sem_unlink(const char*){
   return 0;
}
#endif


/*============================================
| End of Source  : kernel_sem.c
|---------------------------------------------
| Historic:
|---------------------------------------------
| Authors     | Date     | Comments
| $Log: not supported by cvs2svn $
| Revision 1.8  2010/03/11 08:40:38  jjp
| clean kernel_sem_timedwait
|
| Revision 1.7  2010/03/09 13:41:19  jjp
| bug fix if timeout is NULL use cyg_semaphore_wait instead of cyg_semaphore_timed_wait with abstime=0
|
| Revision 1.6  2010/03/09 13:40:49  jjp
| bug fix if timeout is NULL use cyg_semaphore_wait instead of cyg_semaphore_timed_wait with abstime=0
|
| Revision 1.5  2010/03/04 16:42:28  jjp
| remove __kernel_pthread_errno in kernel_sem_wait
|
| Revision 1.4  2010/02/22 09:46:03  jjp
| bug fix : kernel_sem_wait return -1 and set errno to EINTR
|
| Revision 1.3  2010/02/12 14:31:17  jjp
| return right value when interrupted by signal + clean source
|
| Revision 1.2  2009/06/09 10:07:52  jjp
| bug fix on timedwait function
|
| Revision 1.1  2009/03/30 15:48:57  jjp
| first import of tauon
|
| Revision 1.1  2009/03/30 11:18:54  jjp
| First import of tauon
|
| Revision 1.1.1.1  2009/02/05 15:29:49  jjp
| First import in CVS
|
|---------------------------------------------
==============================================*/
