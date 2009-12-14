/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

////////////////////////////////////////////////////////////
//
// Implementation of plugin entry points (NPP_*)
// most are just empty stubs for this particular plugin 
//
#ifdef	XP_WIN32
#include <cstddef>		   	 // Needed for ptrdiff_t. Is used in GeckoSDK 1.9,
//#define ptrdiff_t long int // but not defined in Visual C++ 7.1.
#endif//XP_WIN32

#include "npambulant.h"

char*
NPP_GetMIMEDescription(void)
{
  char* mimetypes = "application/smil:.smi:W3C Smil 3.0 Playable Multimedia file;application/smil+xml:.smil:W3C Smil 3.0 Playable Multimedia file;application/x-ambulant-smil:.smil:W3C Smil 3.0 Ambulant Player compatible file;";
  return mimetypes;
}

npambulant * s_npambulant = NULL;

NPError NPP_Initialize(void)
{
  return NPERR_NO_ERROR;
}

void NPP_Shutdown(void)
{
}

// here the plugin creates an instance of our npambulant object which 
// will be associated with this newly created plugin instance and 
// will do all the neccessary job
NPError NPP_New(NPMIMEType pluginType,
                NPP instance,
                uint16 mode,
                int16 argc,
                char* argn[],
                char* argv[],
                NPSavedData* saved)
{   
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;
#ifdef WITH_CG
	// We need to request CoreGraphics support in stead of QuickDraw support.
	NPBool supportsCG = false;
	rv = NPN_GetValue(instance, NPNVsupportsCoreGraphicsBool, &supportsCG);
	if (rv) {
		AM_DBG fprintf(stderr, "GetValue(NPNVsupportsCoreGraphicsBool) returned %d\n", rv);
		return rv;
	}
	if (!supportsCG) {
		AM_DBG fprintf(stderr, "Browser does not support NPNVsupportsCoreGraphicsBool\n");
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
	rv = NPN_SetValue(instance, NPPVpluginDrawingModel, (void*)NPDrawingModelCoreGraphics);
	if (rv) {
		AM_DBG fprintf(stderr, "SetValue(NPDrawingModelCoreGraphics) returned %d\n", rv);
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	}
#endif
  npambulant * pPlugin = new npambulant(pluginType,instance,mode,argc,argn,argv,saved);
  if(pPlugin == NULL)
    return NPERR_OUT_OF_MEMORY_ERROR;
  s_npambulant = pPlugin;
  instance->pdata = (void *)pPlugin;
  return rv;
}

// here is the place to clean up and destroy the npambulant object
NPError NPP_Destroy (NPP instance, NPSavedData** save)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;

  npambulant * pPlugin = (npambulant *)instance->pdata;
  if(pPlugin != NULL) {
  pPlugin->shut();
    delete pPlugin;
    if (s_npambulant == pPlugin)
      s_npambulant = NULL;
  }
  return rv;
}

// during this call we know when the plugin window is ready or
// is about to be destroyed so we can do some gui specific
// initialization and shutdown
NPError NPP_SetWindow (NPP instance, NPWindow* pNPWindow)
{    
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;

  if(pNPWindow == NULL)
    return NPERR_GENERIC_ERROR;

  npambulant * pPlugin = (npambulant *)instance->pdata;

  if(pPlugin == NULL) 
    return NPERR_GENERIC_ERROR;

  // window just created
  if(!pPlugin->isInitialized() && (pNPWindow->window != NULL)) { 
    if(!pPlugin->init(pNPWindow)) {
      return NPERR_MODULE_LOAD_FAILED_ERROR;
    }
  }

  // window goes away
  if((pNPWindow->window == NULL) && pPlugin->isInitialized())
    return NPERR_NO_ERROR;

  // window resized
  if(pPlugin->isInitialized() && (pNPWindow->window != NULL))
    return NPERR_NO_ERROR;

  // this should not happen, nothing to do
  if((pNPWindow->window == NULL) && !pPlugin->isInitialized())
    return NPERR_NO_ERROR;

  return rv;
}

// ==============================
// ! Scriptability related code !
// ==============================
//
// here the plugin is asked by Mozilla to tell if it is scriptable
// we should return a valid interface id and a pointer to 
// nsScriptablePeer interface which we should have implemented
// and which should be defined in the corressponding *.xpt file
// in the bin/components folder
NPError	NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
  if(instance == NULL || value == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;

  *(NPObject **)value = NULL;

  npambulant * plugin = (npambulant *)instance->pdata;
  if(plugin == NULL)
    return NPERR_GENERIC_ERROR;

  switch (variable) {
  case NPPVpluginNameString:
    *((char **)value) = "NPRuntimeTest";
    break;
  case NPPVpluginDescriptionString:
    *((char **)value) = "NPRuntime scriptability API test plugin";
    break;
  case NPPVpluginScriptableNPObject:
    *(NPObject **)value = plugin->GetScriptableObject();
    break;
  case NPPVpluginNeedsXEmbed:
    *(NPBool *) value = TRUE;
    break;        
  default:
//    rv = NPERR_GENERIC_ERROR;
	break; 
  }

  return rv;
}

NPError NPP_NewStream(NPP instance,
                      NPMIMEType type,
                      NPStream* stream, 
                      NPBool seekable,
                      uint16* stype)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;
  return rv;
}

int32 NPP_WriteReady (NPP instance, NPStream *stream)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  int32 rv = 0x0fffffff;
  return rv;
}

int32 NPP_Write (NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{   
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  int32 rv = len;
  return rv;
}

NPError NPP_DestroyStream (NPP instance, NPStream *stream, NPError reason)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;
  return rv;
}

void NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
  if(instance == NULL)
    return;
}

void NPP_Print (NPP instance, NPPrint* printInfo)
{
  if(instance == NULL)
    return;
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData)
{
  if(instance == NULL)
    return;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
  if(instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  NPError rv = NPERR_NO_ERROR;
  return rv;
}

int16	NPP_HandleEvent(NPP instance, void* event)
{
  if(instance == NULL)
    return 0;

  int16 rv = 0;
  npambulant * pPlugin = (npambulant *)instance->pdata;
  if (pPlugin)
    rv = pPlugin->handleEvent(event);

  return rv;
}

#ifdef OJI
jref NPP_GetJavaClass (void)
{
  return NULL;
}
#endif//OJI

NPObject *NPP_GetScriptableInstance(NPP instance)
{
  if(!instance)
    return 0;

  NPObject *npobj = 0;
  npambulant * pPlugin = (npambulant *)instance->pdata;
  if (!pPlugin)
    npobj = pPlugin->GetScriptableObject();

  return npobj;
}
