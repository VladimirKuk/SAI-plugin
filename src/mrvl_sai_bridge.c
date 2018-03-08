/*
 *  Copyright (C) 2016. Marvell International Ltd. ALL RIGHTS RESERVED.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License"); you may
 *    not use this file except in compliance with the License. You may obtain
 *    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *    THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 *    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *    FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 *    See the Apache Version 2.0 License for specific language governing
 *    permissions and limitations under the License.
 *
 */

#include "sai.h"
#include "mrvl_sai.h"
#include "assert.h"

#undef  __MODULE__
#define __MODULE__ SAI_BRIDGE


static void bridge_id_key_to_str(_In_ sai_object_id_t sai_bridge_id, _Out_ char *key_str);
static void bridge_port_id_key_to_str(_In_ sai_object_id_t sai_bridge_port_id, _Out_ char *key_str);

static sai_status_t mrvl_sai_bridge_type_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg);
static sai_status_t mrvl_sai_bridge_port_list_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
static sai_status_t mrvl_sai_bridge_max_learned_addresses_get(_In_ const sai_object_key_t   *key,
                                                          _Inout_ sai_attribute_value_t *value,
                                                          _In_ uint32_t                  attr_index,
                                                          _Inout_ vendor_cache_t        *cache,
                                                          void                          *arg);
static sai_status_t mrvl_sai_bridge_max_learned_addresses_set(_In_ const sai_object_key_t      *key,
                                                          _In_ const sai_attribute_value_t *value,
                                                          void                             *arg);
static sai_status_t mrvl_sai_bridge_learn_disable_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg);
static sai_status_t mrvl_sai_bridge_learn_disable_set(_In_ const sai_object_key_t      *key,
                                                  _In_ const sai_attribute_value_t *value,
                                                  void                             *arg);

static const sai_attribute_entry_t mrvl_sai_bridge_attribs[] = {
    { SAI_BRIDGE_ATTR_TYPE, true, false, false, true,
      "Bridge type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_BRIDGE_ATTR_PORT_LIST, false, false, false, true,
      "Bridge ports associated to bridge", SAI_ATTR_VAL_TYPE_OBJLIST },
    { SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES, false, false, false, true,
      "Max number of learned MAC addresses", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_BRIDGE_ATTR_LEARN_DISABLE, false, false, false, true,
      "Disable learning on bridge", SAI_ATTR_VAL_TYPE_BOOL },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_bridge_vendor_attribs[] = {
    { SAI_BRIDGE_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_type_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_PORT_LIST,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_bridge_port_list_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_max_learned_addresses_get, NULL,
      mrvl_sai_bridge_max_learned_addresses_set, NULL },
    { SAI_BRIDGE_ATTR_LEARN_DISABLE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_learn_disable_get, NULL,
      mrvl_sai_bridge_learn_disable_set, NULL },
    
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};
static sai_status_t mrvl_sai_bridge_port_type_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
static sai_status_t mrvl_sai_bridge_port_lag_or_port_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg);
static sai_status_t mrvl_sai_bridge_port_vlan_id_get(_In_ const sai_object_key_t   *key,
                                                 _Inout_ sai_attribute_value_t *value,
                                                 _In_ uint32_t                  attr_index,
                                                 _Inout_ vendor_cache_t        *cache,
                                                 void                          *arg);
static sai_status_t mrvl_sai_bridge_port_rif_id_get(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg);
static sai_status_t mrvl_sai_bridge_port_tunnel_id_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg);
static sai_status_t mrvl_sai_bridge_port_bridge_id_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg);
static sai_status_t mrvl_sai_bridge_port_bridge_id_set(_In_ const sai_object_key_t      *key,
                                                   _In_ const sai_attribute_value_t *value,
                                                   void                             *arg);
static sai_status_t mrvl_sai_bridge_port_fdb_learning_mode_get(_In_ const sai_object_key_t   *key,
                                                           _Inout_ sai_attribute_value_t *value,
                                                           _In_ uint32_t                  attr_index,
                                                           _Inout_ vendor_cache_t        *cache,
                                                           void                          *arg);
static sai_status_t mrvl_sai_bridge_port_fdb_learning_mode_set(_In_ const sai_object_key_t      *key,
                                                           _In_ const sai_attribute_value_t *value,
                                                           void                             *arg);
static sai_status_t mrvl_sai_bridge_port_max_learned_addresses_get(_In_ const sai_object_key_t   *key,
                                                               _Inout_ sai_attribute_value_t *value,
                                                               _In_ uint32_t                  attr_index,
                                                               _Inout_ vendor_cache_t        *cache,
                                                               void                          *arg);
static sai_status_t mrvl_sai_bridge_port_max_learned_addresses_set(_In_ const sai_object_key_t      *key,
                                                               _In_ const sai_attribute_value_t *value,
                                                               void                             *arg);
static sai_status_t mrvl_sai_bridge_port_admin_state_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg);
static sai_status_t mrvl_sai_bridge_port_admin_state_set(_In_ const sai_object_key_t      *key,
                                                     _In_ const sai_attribute_value_t *value,
                                                     void                             *arg);

static const sai_attribute_entry_t mrvl_sai_bridge_port_attribs[] = {
    { SAI_BRIDGE_PORT_ATTR_TYPE, true, true, false, true,
      "Bridge port type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_BRIDGE_PORT_ATTR_PORT_ID, true, true, false, true,
      "Bridge port associated port/lag ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_BRIDGE_PORT_ATTR_VLAN_ID, false, true, false, true,
      "Bridge port associated VLAN ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_BRIDGE_PORT_ATTR_RIF_ID, false, true, false, true,
      "Bridge port associated rif ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_BRIDGE_PORT_ATTR_TUNNEL_ID, false, true, false, true,
      "Bridge port associated tunnel ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_BRIDGE_PORT_ATTR_BRIDGE_ID, false, true, false, true,
      "Bridge port associated bridge ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE, false, false, false, true,
      "Bridge port FDB learning mode", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES, false, false, false, true,
      "Bridge port max learned MAC addresses", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION, false, false, false, true,
      "Bridge port action for packets with unkown MAC address", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_BRIDGE_PORT_ATTR_ADMIN_STATE, false, true, false, true,
      "Bridge port admin state", SAI_ATTR_VAL_TYPE_BOOL },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_bridge_port_vendor_attribs[] = {
    { SAI_BRIDGE_PORT_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_type_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_lag_or_port_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_VLAN_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_vlan_id_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_RIF_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_rif_id_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_TUNNEL_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_tunnel_id_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_BRIDGE_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_bridge_id_get, NULL,
      mrvl_sai_bridge_port_bridge_id_set, NULL },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_fdb_learning_mode_get, NULL,
      mrvl_sai_bridge_port_fdb_learning_mode_set, NULL },
    { SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_max_learned_addresses_get, NULL,
      mrvl_sai_bridge_port_max_learned_addresses_set, NULL },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION,
      { true, false, false, false },
      { true, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_ADMIN_STATE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_admin_state_get, NULL,
      mrvl_sai_bridge_port_admin_state_set, NULL },
    
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

/**
 * @brief Bridge type
 *
 * @type sai_bridge_type_t
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 */
static sai_status_t mrvl_sai_bridge_type_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_object_id_t bridge_id   = key->key.object_id;
    uint32_t    bridge_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->s32 = SAI_BRIDGE_TYPE_1Q;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief List of bridge ports associated to this bridge
 *
 * @type sai_object_list_t
 * @objects SAI_OBJECT_TYPE_BRIDGE_PORT
 * @flags READ_ONLY
 */
static sai_status_t mrvl_sai_bridge_port_list_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    sai_object_id_t data_obj;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE_PORT, 1, &data_obj)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_fill_objlist(&data_obj, 1, &value->objlist)))
    {
         MRVL_SAI_LOG_ERR("Failed to fill objlist\n");
         MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Maximum number of learned MAC addresses
 *
 * Zero means learning limit disable
 *
 * @type sai_uint32_t
 * @flags CREATE_AND_SET
 * @default 0
 */
static sai_status_t mrvl_sai_bridge_max_learned_addresses_get(_In_ const sai_object_key_t   *key,
                                                          _Inout_ sai_attribute_value_t *value,
                                                          _In_ uint32_t                  attr_index,
                                                          _Inout_ vendor_cache_t        *cache,
                                                          void                          *arg)
{
    sai_object_id_t bridge_id   = key->key.object_id;
    uint32_t    bridge_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->u32 = 0;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Maximum number of learned MAC addresses
 *
 * Zero means learning limit disable
 *
 * @type sai_uint32_t
 * @flags CREATE_AND_SET
 * @default 0
 */
static sai_status_t mrvl_sai_bridge_max_learned_addresses_set(_In_ const sai_object_key_t      *key,
                                                          _In_ const sai_attribute_value_t *value,
                                                          void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief To disable learning on a bridge
 *
 * @type bool
 * @flags CREATE_AND_SET
 * @default false
 */
static sai_status_t mrvl_sai_bridge_learn_disable_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg)
{
    sai_object_id_t bridge_id   = key->key.object_id;
    uint32_t    bridge_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->booldata = false;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief To disable learning on a bridge
 *
 * @type bool
 * @flags CREATE_AND_SET
 * @default false
 */
static sai_status_t mrvl_sai_bridge_learn_disable_set(_In_ const sai_object_key_t      *key,
                                                  _In_ const sai_attribute_value_t *value,
                                                  void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static void bridge_id_key_to_str(_In_ sai_object_id_t sai_bridge_id, _Out_ char *key_str)
{
    uint32_t     bridge_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_bridge_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid bridge id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "bridge id %u", bridge_id);
    }
}

static void bridge_port_id_key_to_str(_In_ sai_object_id_t sai_bridge_port_id, _Out_ char *key_str)
{
    uint32_t     bridge_port_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid bridge port id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "bridge port id %u", bridge_port_id);
    }
}

/**
 * @brief Create bridge
 *
 * @param[out] bridge_id Bridge ID
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_bridge(
        _Out_ sai_object_id_t* bridge_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status;
    char                         key_str[MAX_KEY_STR_LEN];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (NULL == bridge_id) {
        MRVL_SAI_LOG_ERR("NULL bridge id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_bridge_attribs, mrvl_sai_bridge_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_bridge_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", list_str);

    /* create SAI BRIDGE object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE, 1, bridge_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove bridge
 *
 * @param[in] bridge_id Bridge ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_bridge (
        _In_ sai_object_id_t bridge_id)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set attribute for bridge
 *
 * @param[in] bridge_id Bridge ID
 * @param[in] attr Attribute to set
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_set_bridge_attribute(
        _In_ sai_object_id_t bridge_id,
        _In_ const sai_attribute_t *attr)
{
	const sai_object_key_t key = { .key.object_id = bridge_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    bridge_id_key_to_str(bridge_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_bridge_attribs, mrvl_sai_bridge_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get attributes of bridge
 *
 * @param[in] bridge_id Bridge ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_get_bridge_attribute(
        _In_ sai_object_id_t bridge_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
	const sai_object_key_t key = { .key.object_id = bridge_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == bridge_id) {
        MRVL_SAI_LOG_ERR("NULL bridge id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    bridge_id_key_to_str(bridge_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_bridge_attribs, mrvl_sai_bridge_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);    
}

/**
 * @brief Bridge port type
 *
 * @type sai_bridge_port_type_t
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 */
static sai_status_t mrvl_sai_bridge_port_type_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }

    value->s32 = SAI_BRIDGE_PORT_TYPE_PORT;
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated Port or Lag object id
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_PORT, SAI_OBJECT_TYPE_LAG
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 * @condition SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_PORT or SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_SUB_PORT
 */
static sai_status_t mrvl_sai_bridge_port_lag_or_port_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object port\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated Vlan
 *
 * @type sai_uint16_t
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 * @condition SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_SUB_PORT
 * @isvlan true
 */
static sai_status_t mrvl_sai_bridge_port_vlan_id_get(_In_ const sai_object_key_t   *key,
                                                 _Inout_ sai_attribute_value_t *value,
                                                 _In_ uint32_t                  attr_index,
                                                 _Inout_ vendor_cache_t        *cache,
                                                 void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }

    value->u16 = 1;
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated router inerface object id
 * Please note that for SAI_BRIDGE_PORT_TYPE_1Q_ROUTER,
 * all vlan interfaces are auto bounded for the bridge port.
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_ROUTER_INTERFACE
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 * @condition SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_1D_ROUTER
 */
static sai_status_t mrvl_sai_bridge_port_rif_id_get(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object rif\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated tunnel id
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_TUNNEL
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 * @condition SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_TUNNEL
 */
static sai_status_t mrvl_sai_bridge_port_tunnel_id_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_TUNNEL, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object tunnel\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated bridge id
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_BRIDGE
 * @flags CREATE_AND_SET
 * @default SAI_NULL_OBJECT_ID
 * @allownull true
 */
static sai_status_t mrvl_sai_bridge_port_bridge_id_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated bridge id
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_BRIDGE
 * @flags CREATE_AND_SET
 * @default SAI_NULL_OBJECT_ID
 * @allownull true
 */
static sai_status_t mrvl_sai_bridge_port_bridge_id_set(_In_ const sai_object_key_t      *key,
                                                   _In_ const sai_attribute_value_t *value,
                                                   void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief FDB Learning mode
 *
 * @type sai_bridge_port_fdb_learning_mode_t
 * @flags CREATE_AND_SET
 * @default SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW
 */
static sai_status_t mrvl_sai_bridge_port_fdb_learning_mode_get(_In_ const sai_object_key_t   *key,
                                                           _Inout_ sai_attribute_value_t *value,
                                                           _In_ uint32_t                  attr_index,
                                                           _Inout_ vendor_cache_t        *cache,
                                                           void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief FDB Learning mode
 *
 * @type sai_bridge_port_fdb_learning_mode_t
 * @flags CREATE_AND_SET
 * @default SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW
 */
static sai_status_t mrvl_sai_bridge_port_fdb_learning_mode_set(_In_ const sai_object_key_t      *key,
                                                           _In_ const sai_attribute_value_t *value,
                                                           void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Maximum number of learned MAC addresses
 *
 * @type sai_uint32_t
 * @flags CREATE_AND_SET
 * @default 0
 */
static sai_status_t mrvl_sai_bridge_port_max_learned_addresses_get(_In_ const sai_object_key_t   *key,
                                                               _Inout_ sai_attribute_value_t *value,
                                                               _In_ uint32_t                  attr_index,
                                                               _Inout_ vendor_cache_t        *cache,
                                                               void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->u32 = 0;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Maximum number of learned MAC addresses
 *
 * @type sai_uint32_t
 * @flags CREATE_AND_SET
 * @default 0
 */
static sai_status_t mrvl_sai_bridge_port_max_learned_addresses_set(_In_ const sai_object_key_t      *key,
                                                               _In_ const sai_attribute_value_t *value,
                                                               void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Admin Mode.
 *
 * Before removing a bridge port, need to disable it by setting admin mode
 * to false, then flush the FDB entries, and then remove it.
 *
 * @type bool
 * @flags CREATE_AND_SET
 * @default false
 */
static sai_status_t mrvl_sai_bridge_port_admin_state_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    uint32_t    bridge_port_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->booldata = false;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_bridge_port_admin_state_set(_In_ const sai_object_key_t      *key,
                                                     _In_ const sai_attribute_value_t *value,
                                                     void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Create bridge port
 *
 * @param[out] bridge_port_id Bridge port ID
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_bridge_port(
        _Out_ sai_object_id_t* bridge_port_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status;
    char                         key_str[MAX_KEY_STR_LEN];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (NULL == bridge_port_id) {
        MRVL_SAI_LOG_ERR("NULL bridge port id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_bridge_port_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", list_str);

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_bridge_port_attribs, mrvl_sai_bridge_port_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* create SAI BRIDGE PORT object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE_PORT, 1, bridge_port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_bridge_port (
        _In_ sai_object_id_t bridge_port_id)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set attribute for bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 * @param[in] attr Attribute to set
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_set_bridge_port_attribute(
        _In_ sai_object_id_t bridge_port_id,
        _In_ const sai_attribute_t *attr)
{
	const sai_object_key_t key = { .key.object_id = bridge_port_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    bridge_port_id_key_to_str(bridge_port_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_bridge_port_attribs, mrvl_sai_bridge_port_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get attributes of bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_get_bridge_port_attribute(
        _In_ sai_object_id_t bridge_port_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
	const sai_object_key_t key = { .key.object_id = bridge_port_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == bridge_port_id) {
        MRVL_SAI_LOG_ERR("NULL bridge port id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    bridge_port_id_key_to_str(bridge_port_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_bridge_port_attribs, mrvl_sai_bridge_port_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status); 
}

/**
 * @brief Bridge methods table retrieved with sai_api_query()
 */
const sai_bridge_api_t bridge_api =
{
    mrvl_sai_create_bridge,
    mrvl_sai_remove_bridge,
    mrvl_sai_set_bridge_attribute,
    mrvl_sai_get_bridge_attribute,
    mrvl_sai_create_bridge_port,
    mrvl_sai_remove_bridge_port,
    mrvl_sai_set_bridge_port_attribute,
    mrvl_sai_get_bridge_port_attribute
};