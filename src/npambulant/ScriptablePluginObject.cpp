#include "ScriptablePluginObject.h"
#include "ConstructablePluginObject.h"
#include "CPlugin.h"

static NPObject *
AllocateConstructablePluginObject(NPP npp, NPClass *aClass)
{
  return new ConstructablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ConstructablePluginObject,
                                 AllocateConstructablePluginObject);

bool
ConstructablePluginObject::Construct(const NPVariant *args, uint32_t argCount,
                                     NPVariant *result)
{
  printf("Creating new ConstructablePluginObject!\n");

  NPObject *myobj =
    NPN_CreateObject(mNpp, GET_NPOBJECT_CLASS(ConstructablePluginObject));
  if (!myobj)
    return false;

  OBJECT_TO_NPVARIANT(myobj, *result);

  return true;
}
NPObject *
AllocateScriptablePluginObject(NPP npp, NPClass *aClass)
{
  return new ScriptablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject,
                                 AllocateScriptablePluginObject);

bool
ScriptablePluginObject::HasMethod(NPIdentifier name)
{
  return name == sFoo_id
	|| name == sStartPlayer_id
	|| name == sStopPlayer_id
	|| name == sRestartPlayer_id
	|| name == sPausePlayer_id
	|| name == sResumePlayer_id
	|| name == sIsDone_id
	;
}

bool
ScriptablePluginObject::HasProperty(NPIdentifier name)
{
  return (name == sPluginType_id);
}

bool
ScriptablePluginObject::GetProperty(NPIdentifier name, NPVariant *result)
{
  VOID_TO_NPVARIANT(*result);

  if (name == sPluginType_id) {
    NPObject *myobj =
      NPN_CreateObject(mNpp, GET_NPOBJECT_CLASS(ConstructablePluginObject));
    if (!myobj) {
      return false;
    }

    OBJECT_TO_NPVARIANT(myobj, *result);

    return true;
  }

  return true;
}

bool
ScriptablePluginObject::Invoke(NPIdentifier name, const NPVariant *args,
                               uint32_t argCount, NPVariant *result)
{
  if (name == sFoo_id) {
    printf ("foo called!\n");

    NPVariant docv;
    NPN_GetProperty(mNpp, sWindowObj, sDocument_id, &docv);

    NPObject *doc = NPVARIANT_TO_OBJECT(docv);

    NPVariant strv;
    STRINGZ_TO_NPVARIANT("div", strv);

    NPVariant divv;
    NPN_Invoke(mNpp, doc, sCreateElement_id, &strv, 1, &divv);

    STRINGZ_TO_NPVARIANT("I'm created by a plugin!", strv);

    NPVariant textv;
    NPN_Invoke(mNpp, doc, sCreateTextNode_id, &strv, 1, &textv);

    NPVariant v;
    NPN_Invoke(mNpp, NPVARIANT_TO_OBJECT(divv), sAppendChild_id, &textv, 1,
               &v);
    NPN_ReleaseVariantValue(&v);

    NPN_ReleaseVariantValue(&textv);

    NPVariant bodyv;
    NPN_GetProperty(mNpp, doc, sBody_id, &bodyv);

    NPN_Invoke(mNpp, NPVARIANT_TO_OBJECT(bodyv), sAppendChild_id, &divv, 1,
               &v);
    NPN_ReleaseVariantValue(&v);

    NPN_ReleaseVariantValue(&divv);
    NPN_ReleaseVariantValue(&bodyv);

    NPN_ReleaseVariantValue(&docv);

    STRINGZ_TO_NPVARIANT(strdup("foo return val"), *result);

    return PR_TRUE;
  }
  if (name == sStartPlayer_id) {
    printf ("startPlayer called !\n");
	if (mNpp)
	  ((CPlugin*)mNpp->pdata)->startPlayer();
    STRINGZ_TO_NPVARIANT(strdup("startPlayer return val"), *result);

    return PR_TRUE;

  } else  if (name == sStopPlayer_id) {
    printf ("stopPlayer called !\n");
	if (mNpp)
	  ((CPlugin*)mNpp->pdata)->stopPlayer();
    STRINGZ_TO_NPVARIANT(strdup("stopPlayer return val"), *result);

    return PR_TRUE;

  } else  if (name == sRestartPlayer_id) {
    printf ("restartPlayer called !\n");
	if (mNpp)
	  ((CPlugin*)mNpp->pdata)->restartPlayer();
    STRINGZ_TO_NPVARIANT(strdup("restartPlayer return val"), *result);

    return PR_TRUE;

  } else  if (name == sPausePlayer_id) {
    printf ("pausePlayer called !\n");
	if (mNpp)
	  ((CPlugin*)mNpp->pdata)->pausePlayer();
    STRINGZ_TO_NPVARIANT(strdup("pausePlayer return val"), *result);

    return PR_TRUE;

  } else  if (name == sResumePlayer_id) {
    printf ("resumePlayer called !\n");
	if (mNpp)
	  ((CPlugin*)mNpp->pdata)->resumePlayer();
    STRINGZ_TO_NPVARIANT(strdup("resumePlayer return val"), *result);

    return PR_TRUE;

  } else  if (name == sIsDone_id) {
    printf ("isDone called !\n");
	if (mNpp) {
	  bool rv = ((CPlugin*)mNpp->pdata)->isDone();
	  BOOLEAN_TO_NPVARIANT(rv, *result);
	  return PR_TRUE;
	}
  }
  return PR_FALSE;
}

bool
ScriptablePluginObject::InvokeDefault(const NPVariant *args, uint32_t argCount,
                                      NPVariant *result)
{
  printf ("ScriptablePluginObject default method called!\n");

  STRINGZ_TO_NPVARIANT(strdup("default method return val"), *result);

  return PR_TRUE;
}
