/**=============================================================================

@file
scveContext.hpp

@brief
SCVE API Definition containing SCVE Context related definitions.

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/

//=============================================================================
///@defgroup scveContext Context
///@brief Defines API for SCVE-Context
///@ingroup scve
//=============================================================================

#ifndef SCVE_CONTEXT_HPP
#define SCVE_CONTEXT_HPP

#include "scveTypes.hpp"

namespace SCVE
{

//------------------------------------------------------------------------------
/// @brief
///    OperationMode enum defines modes which could be optionally assigned
///    to every Context object during it's initialization.
///
/// @see Context::newInstance()
///
/// @ingroup scveContext
//------------------------------------------------------------------------------
typedef enum
{
   /// Default mode in which a Context will be initialized.
   SCVE_MODE_DEFAULT            = 0,
   /// Performance mode, where the fastest implementation is selected for all
   /// SCVE features initialized under the context.
   SCVE_MODE_PERFORMANCE        = 1,
   /// CPU Offload mode, where preference is given to offload the implementation
   /// to any available hardware / dsp.
   SCVE_MODE_CPU_OFFLOAD        = 2,
   /// Power Save mode, where preference is given to an implementation that
   /// draws least power.
   SCVE_MODE_LOW_POWER          = 3,
}OperationMode;

//------------------------------------------------------------------------------
/// @brief
///    Context class implements the context-object that is associated with
///    every SCVE session.
///
/// @details
///    - A pre-requisite to creating a new instance of any SCVE feature is to
///    have an appropriate context for it to execute in. This class implements
///    that context object, which should be used to initialize a SCVE session
///    in appropriate operation-mode \n
///    - Once a new Context is created with a specific operation-mode, it cannot
///    be changed for that context object. If you need two or more SCVE features
///    running in different operation modes, you will need to create as many
///    context objects and initialize each SCVE feature with appropriate context
///    - Never de-initialize a context before all the associated features are
///    de-initialized. Doing so can have undesirable consequences.
///
///    \b Example: (one session object)
/// @code
///    #include <scveTypes.hpp>
///    #include <scveContext.hpp>
///    //#include for other feature header files
///    ...
///    //First, initialize a context object
///    SCVE::Context *pContext = SCVE::Context::newInstance(SCVE::SCVE_MODE_PERFORMANCE);
///    //Then, initialize any SCVE feature using the context object
///    ...
///    // Use the SCVE feature
///    ...
///    //De-initialize all features before you de-initialize context
///    ...
///    //Now de-initialize the context itself
///    delete pContext;
/// @endcode
///
/// @ingroup scveContext
//------------------------------------------------------------------------------
class SCVE_API Context
{
   public:
      //------------------------------------------------------------------------------
      /// @brief
      ///    Creates a new instance of a Context with the optionally provided
      ///    operation mode parameter.
      ///
      /// @details
      ///    The mode is sticky for this context object - that is, once set
      ///    it cannot be changed. OperationMode is an optional parameter, in absence
      ///    of which, the default mode is chosen.
      ///
      /// @param mode
      ///    [optional] Parameter that specifies which operation mode to execute
      ///    under.
      ///
      /// @retval Non-Zero if context initialization is successful.
      /// @retval Zero (0 or NULL) if context initialization has failed.
      //------------------------------------------------------------------------------
      static Context* newInstance(SCVE::OperationMode mode = SCVE_MODE_DEFAULT);

      static Status deleteInstance(Context *pContext);

   protected:
      //------------------------------------------------------------------------------
      /// @brief
      ///    Protected constructor of Context object.
      ///
      /// @details
      ///    - The default constructor is protected for implementation reasons. To
      ///    create a new context, please use newInstance() function.
      //------------------------------------------------------------------------------
      Context();

      //------------------------------------------------------------------------------
      /// @brief
      ///    Protected destructor of Context object.
      ///
      /// @details
      ///    - The default destructor is protected for implementation reasons. To
      ///    cleanup/delete a context object, please use deleteInstance() function.
      //------------------------------------------------------------------------------
      virtual ~Context();
};

} //namespace SCVE

#endif //SCVE_CONTEXT_HPP