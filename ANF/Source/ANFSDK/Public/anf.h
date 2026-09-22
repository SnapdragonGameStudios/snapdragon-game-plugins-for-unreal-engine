//============================================================================================================
//
//                  Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

/**
********************************************************************************************************************************
* @file  anf.h
* @brief ANF API Header
********************************************************************************************************************************
*/

#ifndef ANF_H
#define ANF_H

#ifdef __cplusplus
    #ifndef ANF_ENTRY
        #if defined(_WIN32) || defined(_WIN64)
            #define ANF_ENTRY __declspec(dllimport)
        #elif defined(__ANDROID__) || defined (_LINUX)
            #define ANF_ENTRY __attribute__((visibility ("default")))
        #else
            // Other platforms not supported
        #endif
    #endif
#endif

#ifndef ANF_ENTRY
    #define ANF_ENTRY
#endif

#include "anf_sr.h"
#include "anf_fg.h"
#include "anf_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * AnfQueryAnfVersion
 *
 * Query the version of the ANF implementation
 *
 * @param pVersion Returned version of the ANF implementation
 *
 *
 * @return AnfResult result of query operation
 */
AnfResult AnfQueryAnfVersion(
    AnfVersion*                         pVersion);          ///< Returned version of the ANF implementation

typedef AnfResult (*PfnAnfQueryAnfVersion)(
    AnfVersion*                         pVersion);          ///< Returned version of the ANF implementation

/**
 * AnfCreateInstance
 *
 * Create the ANF instance
 *
 * @param pInstance   Pointer to the created instance handle
 * @param pCreateInfo Create info for the instance
 *
 * @return AnfResult result of creating the instance
 */
AnfResult AnfCreateInstance(
    const AnfInstanceCreateInfo* pCreateInfo,        ///< Create info for instance
    AnfInstance*                 pInstance);         ///< Pointer to the created instance handle

typedef AnfResult (*PfnAnfCreateInstance)(
    const AnfInstanceCreateInfo* pCreateInfo,        ///< Create info for instance
    AnfInstance*                 pInstance);         ///< Pointer to the created instance handle

/**
 * AnfDestroyInstance
 *
 * Destroy the ANF instance
 *
 * @param instance Instance to destroy 
 *
 *
 * @return AnfResult result of destroy operation
 */
AnfResult AnfDestroyInstance(
    AnfInstance                        instance);          ///< Instance to destroy

typedef AnfResult (*PfnAnfDestroyInstance)(
    AnfInstance                        instance);          ///< Instance to destroy

/**
 * AnfSetInstanceClientAPIAdapterInfo
 * 
 * Sets the client API device info for the ANF instance
 *  
 * This MUST be called before any calls to AnfCreateTechnique, AnfDestroyTechnique, and AnfDispatchTechnique
 * 
 * An AnfInstance can only be associated with a single client API device
 * 
 * This lets the ANF instance know the client API handles for making internal client API calls
 * 
 * @param instance             AnfInstance handle for which to set the client API device info.
 * @param pClientApiDeviceInfo Pointer to a struct describing the device info for the given client API
 *                             For Vulkan the struct must be AnfAdapterInfoVulkan.
 * 
 * @return AnfResult result of validating the device info
 */
AnfResult AnfSetInstanceClientAPIAdapterInfo(
    AnfInstance             instance,              ///< ANF Instance handle
    const AnfStructHeader*  pClientApiDeviceInfo); ///< Pointer to struct for given client API

typedef AnfResult (*PfnAnfSetInstanceClientAPIAdapterInfo)(
    AnfInstance             instance,              ///< ANF Instance handle
    const AnfStructHeader*  pClientApiDeviceInfo); ///< Pointer to struct for given client API

/**
 * AnfQueryTechniques
 *
 * Query what techniques are supported by this instance
 *
 * @param ppTechniqueIds Will update to point to array of technique IDs
 * @param ppTechniqueIds Will update variable at address to the number of techniques
 * @param instance       Instance handle
 *
 * @return AnfResult result of query operation
 */
AnfResult AnfQueryTechniques(
    AnfInstance                        instance,           ///< Instance handle
    uint32_t*                          pNumTechniques,     ///< Will set *pNumTechniques to number of techniques
    AnfTechniqueId**                   ppTechniqueIds);    ///< Will point *ppTechniques to array of technique IDs

typedef AnfResult (*PfnAnfQueryTechniques)(
    AnfInstance                        instance,           ///< Instance handle
    uint32_t*                          pNumTechniques,     ///< Will set *pNumTechniques to number of techniques
    AnfTechniqueId**                   ppTechniqueIds);    ///< Will point *ppTechniques to array of technique IDs

/**
 * AnfQueryTechniqueRequirements
 *
 * Query the requirements for the given technique
 *
 * @param ppTechniqueRequirements Where to set the address of the returned data
 * @param instance                Instance handle
 * @param techniqueId             Id of technique to query requirements of
 *
 * @return AnfResult result of query operation
 */
AnfResult AnfQueryTechniqueRequirements(
    AnfInstance                        instance,                  ///< Instance handle
    AnfTechniqueId                     techniqueId,               ///< Technique ID
    const AnfTechniqueRequirements**   ppTechniqueRequirements);  ///< *ppTechniqueRequirements set to address of returned data

typedef AnfResult (*PfnAnfQueryTechniqueRequirements)(
    AnfInstance                        instance,                  ///< Instance handle
    AnfTechniqueId                     techniqueId,               ///< Technique ID
    const AnfTechniqueRequirements**   ppTechniqueRequirements);  ///< *ppTechniqueRequirements set to address of returned data

/**
 * AnfIsTechniqueSupported
 *
 * Indicates whether the technique is supported for the given adapter.
 *
 * @param instance      A valid AnfInstance.
 * @param techniqueId   ID of the technique we are checking support for.
 * @param pAdapterInfo  Must point to an Anf struct that contains adapter
 *                      info as indicated in the ANF spec.
 *                      For Vulkan, this must be AnfAdapterInfoVulkan.
 *
 *
 * @return AnfResult    Will indicate whether the technique is supported or
 *                      not for the given ANF instance.
 *                      Explanation of some return values:
 *                      - ANF_RESULT_SUCCESS       - the technique is supported
 *                      - ANF_RESULT_NOT_SUPPORTED - the technique is not supported
 *                      - An error return type      - the corresponding error occurred
 */
AnfResult AnfIsTechniqueSupported(
    AnfInstance             instance,                               ///< ANF instance
    AnfTechniqueId          techniqueId,                            ///< Technique ID
    const AnfStructHeader*  pAdapterInfo);                          ///< Adapter info

typedef AnfResult (*PfnAnfIsTechniqueSupported)(
    AnfInstance             instance,                               ///< ANF instance
    AnfTechniqueId          techniqueId,                            ///< Technique ID
    const AnfStructHeader*  pAdapterInfo);                          ///< Adapter info

/**
 * AnfQueryTechniqueResourceRequirements
 *
 * For a given technique, query the requirements for a resource, e.g. supported resource formats
 *
 * @param ppResourceRequirements Where to set to the address of returned data
 * @param instance               Instance handle
 * @param techniqueId            Id of technique to query resource requirements from
 * @param resourceLabel          Lable of resource to query requirements of
 *
 *
 * @return AnfResult result of query operation
 */
AnfResult AnfQueryTechniqueResourceRequirements(
    AnfInstance                        instance,                   ///< Instance handle
    AnfTechniqueId                     techniqueId,                ///< Technique ID
    AnfResourceLabel                   resourceLabel,              ///< Label for resource we're querying about
    const AnfResourceRequirements**    ppResourceRequirements);    ///< *ppResourceRequirements set to address of returned data

typedef AnfResult (*PfnAnfQueryTechniqueResourceRequirements)(
    AnfInstance                        instance,                   ///< Instance handle
    AnfTechniqueId                     techniqueId,                ///< Technique ID
    AnfResourceLabel                   resourceLabel,              ///< Label for resource we're querying about
    const AnfResourceRequirements**    ppResourceRequirements);    ///< *ppResourceRequirements set to address of returned data

/**
 * AnfCreateTechnique
 *
 * Creates a technique
 *
 * @param pTechnique  Pointer to the created technique handle
 * @param instance    Instance handle
 * @param pCreateInfo Create info for new technique
 *
 *
 * @return AnfResult result of create operation
 */
AnfResult AnfCreateTechnique(
    AnfInstance                     instance,                   ///< Instance handle
    const AnfTechniqueCreateInfo*   pCreateInfo,                ///< Create info
    AnfTechnique*                   pTechnique);                ///< Pointer to the created technique handle

typedef AnfResult (*PfnAnfCreateTechnique)(
    AnfInstance                     instance,                   ///< Instance handle
    const AnfTechniqueCreateInfo*   pCreateInfo,                ///< Create info
    AnfTechnique*                   pTechnique);                ///< Pointer to the created technique handle

/**
 * AnfDispatchTechnique
 *
 * Dispatches a technique
 *
 * @param technique     Handle of technique to dispatch
 * @param pDispatchInfo Pointer to Dispatch Info
 *
 *
 * @return AnfResult result of dispatch operation
 */
AnfResult AnfDispatchTechnique(
    AnfTechnique                      technique,                    ///< Technique handle
    const AnfTechniqueDispatchInfo*   pDispatchInfo);               ///< Dispatch info

typedef AnfResult (*PfnAnfDispatchTechnique)(
    AnfTechnique                      technique,                    ///< Technique handle
    const AnfTechniqueDispatchInfo*   pDispatchInfo);               ///< Dispatch info

/**
 * AnfDestroyTechnique
 *
 * Destroy an AnfTechnique
 *
 * @param technique Handle of technique to destroy
 * @param instance  Handle of instance used to create the technique
 *
 * @return AnfResult result of destroy operation
 */
AnfResult AnfDestroyTechnique(
    AnfInstance                       instance,                     ///< Instance handle
    AnfTechnique                      technique);                   ///< Technique handle

typedef AnfResult (*PfnAnfDestroyTechnique)(
    AnfInstance                       instance,                     ///< Instance handle
    AnfTechnique                      technique);                   ///< Technique handle

// The standard types etc. for the ANF implementation interface
struct AnfFunctions
{
    // Main interface
    PfnAnfQueryAnfVersion                    QueryAnfVersion;
    PfnAnfCreateInstance                     CreateInstance;
    PfnAnfDestroyInstance                    DestroyInstance;
    PfnAnfSetInstanceClientAPIAdapterInfo    SetInstanceClientAPIAdapterInfo;
    PfnAnfIsTechniqueSupported               IsTechniqueSupported;
    PfnAnfQueryTechniqueRequirements         QueryTechniqueRequirements;
    PfnAnfQueryTechniqueResourceRequirements QueryTechniqueResourceRequirements;
    PfnAnfCreateTechnique                    CreateTechnique;
    PfnAnfDestroyTechnique                   DestroyTechnique;
    PfnAnfDispatchTechnique                  DispatchTechnique;
};

/*
    Expected procedure is to get the function pointers with GetAnfFunctions, and
    then use those functions to use the interface.

    For example:

    AnfFunctions m_anf = {};

    GetAnfFunctions(&m_anf);

    m_anf.CreateInstance(...);
    m_anf.CreateTechnique(...);
 */

/**
 * GetAnfFunctions
 *
 * Call this to get the functions
 *
 * @param pFunctions pointer to AnfFunctions struct to fill function pointers
 *
 * @return None
 */
ANF_ENTRY void GetAnfFunctions(
    AnfFunctions* pFunctions);

#ifdef __cplusplus
}
#endif

#endif // ANF_H