
/*!
  @file
  qcril_qmi_singleton_agent.h

  @brief
  An abstract template class for when we have a singleton agent thread running for
  a specific dedicated task, e.g. a socket receiver or a sysfs state monitor

*/

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef QCRIL_QMI_SINGLETON_AGENT_H
#define QCRIL_QMI_SINGLETON_AGENT_H

#include <pthread.h>
#include <qcrili.h>

template <typename T>
class qcril_qmi_singleton_agent
{
public:
   static T* get_instance();
   int init();
   int init(char *);
   boolean is_inited() { return ( FALSE != inited); }

protected:
   qcril_qmi_singleton_agent(const char* agent_thread_name = NULL);
   virtual ~qcril_qmi_singleton_agent() {}
#ifdef QMI_RIL_UTF
   void reset_state_reboot();
#endif

private:
   int create_thread();
   static void* thread_func_wrapper(void* this_ptr);
   virtual void thread_func() = 0;
   qcril_qmi_singleton_agent( const qcril_qmi_singleton_agent& rhs );
   qcril_qmi_singleton_agent& operator=( const qcril_qmi_singleton_agent& rhs);


private: // data member
   static T* instance;
   boolean inited;
   pthread_t thread_id;
   char thread_name_str[QMI_RIL_THREAD_NAME_MAX_SIZE];
};

template<class T>
T* qcril_qmi_singleton_agent<T>::instance = NULL;

//===========================================================================
// qcril_qmi_singleton_agent<T>::qcril_qmi_singleton_agent
//===========================================================================
template <class T>
qcril_qmi_singleton_agent<T>::qcril_qmi_singleton_agent(const char* agent_thread_name) : inited (0)
{
   if (agent_thread_name)
   {
      strlcpy(thread_name_str, agent_thread_name, sizeof(thread_name_str));
   }
   else
   {
      thread_name_str[0] = 0;
   }
} // qcril_qmi_singleton_agent<T>::qcril_qmi_singleton_agent

//===========================================================================
// qcril_qmi_singleton_agent<T>::get_instance
//===========================================================================
template <class T>
T* qcril_qmi_singleton_agent<T>::get_instance()
{
   if (NULL == instance)
   {
      instance = new T;
   }
   return instance;
} // qcril_qmi_singleton_agent<T>::get_instance

//===========================================================================
// qcril_qmi_singleton_agent<T>::init
//===========================================================================
template <class T>
int qcril_qmi_singleton_agent<T>::init()
{
   int ret = 0;

   if (!inited)
   {
      QCRIL_LOG_INFO("init agent..");
      if (0 == create_thread())
      {
         inited = TRUE;
      }
      else
      {
         QCRIL_LOG_DEBUG("agent is already inited before..");
         ret = -1;
      }
   }
   QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
   return ret;
} // qcril_qmi_singleton_agent<T>::init

//===========================================================================
// qcril_qmi_singleton_agent<T>::init
//===========================================================================
template <class T>
int qcril_qmi_singleton_agent<T>::init(char* agent_thread_name)
{
   int ret = 0;

   if (agent_thread_name)
   {
      QCRIL_LOG_INFO("init: Create thread %s", agent_thread_name);
      strlcpy(thread_name_str, agent_thread_name, sizeof(thread_name_str));
   }
   else
   {
      thread_name_str[0] = 0;
   }

   if (!inited)
   {
      QCRIL_LOG_INFO("init agent..");
      if (0 == create_thread())
      {
         inited = TRUE;
      }
      else
      {
         QCRIL_LOG_DEBUG("agent is already inited before..");
         ret = -1;
      }
   }
   QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
   return ret;
} // qcril_qmi_singleton_agent<T>::init

//===========================================================================
// qcril_qmi_singleton_agent<T>::create_thread
//===========================================================================
template <class T>
int qcril_qmi_singleton_agent<T>::create_thread()
{
    int ret = 0;
    QCRIL_LOG_FUNC_ENTRY();
    pthread_attr_t attr;
#ifdef QMI_RIL_UTF
    pthread_attr_init (&attr);
    if ( 0 == utf_pthread_create_handler(&thread_id, &attr, qcril_qmi_singleton_agent<T>::thread_func_wrapper, this) )
    {
#else
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if ( 0 == pthread_create(&thread_id, &attr, qcril_qmi_singleton_agent<T>::thread_func_wrapper, this) )
    {
#endif
       if (strlen(thread_name_str))
       {
           qmi_ril_set_thread_name(thread_id, thread_name_str);
       }
    }
    else
    {
        QCRIL_LOG_ERROR("Error in pthread_create");
        ret = -1;
    }
    pthread_attr_destroy(&attr);

    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
} // qcril_qmi_singleton_agent<T>::create_thread

//===========================================================================
// qcril_qmi_singleton_agent<T>::thread_func_wrapper
//===========================================================================
template <class T>
void* qcril_qmi_singleton_agent<T>::thread_func_wrapper(void* this_ptr)
{
    ((qcril_qmi_singleton_agent<T> *) this_ptr)->thread_func();
    return NULL;
} // qcril_qmi_singleton_agent<T>::thread_func_wrapper


#ifdef QMI_RIL_UTF
//===========================================================================
// qcril_qmi_singleton_agent<T>::reset_state_reboot
//===========================================================================
template <class T>
void qcril_qmi_singleton_agent<T>::reset_state_reboot()
{
  inited = 0;
}
#endif

#endif /* QCRIL_QMI_SINGLETON_AGENT_H */
