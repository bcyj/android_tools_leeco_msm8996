#ifndef __LOG_H__
#define __LOG_H__

/* To enable logging uncomment the line below */
#define QMI_TEST_LOGGING_ENABLED

#ifdef QMI_TEST_LOGGING_ENABLED
#define QMI_TEST_LOG1(a) printf(a)
#define QMI_TEST_LOG2(a, b) printf(a, b)
#define QMI_TEST_LOG3(a, b, c) printf(a, b, c)
#else
#define QMI_TEST_LOG1(a)
#define QMI_TEST_LOG2(a, b) 
#define QMI_TEST_LOG3(a, b, c) 
#endif


/* Extry Trace */
#define ENTRYTRACE QMI_TEST_LOG3("Entered %s Function, line number: %d\n",\
 __FUNCTION__,__LINE__);

/* Exit Trace */
#define EXITTRACE QMI_TEST_LOG3("Exited %s Function, line number: %d\n",\
 __FUNCTION__,__LINE__);

 
 
#endif
