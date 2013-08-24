<?php

/**
 * Node field form.
 */
function node_field_node_field_form($form, &$form_state, $node) {
  $form_state['wrapper_id'] = 'node-field-node-field-form';
  $form_state['node'] = $node;
  $node_fields = node_field_get_node_fields($node);
  $form_state['node_fields'] = $node_fields;
  $node_edit_page = l(t('node edit page'), 'node/' . $node->nid . '/edit');
  $form['description'] = array(
    '#type' => 'markup',
    '#markup' => t('Configure Node Fields here. Then set values on !link.', array('!link' => $node_edit_page)),
  );
  // Building node fields form.
  $form['node_fields'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => $form_state['wrapper_id'],
    ),
    '#tree' => TRUE,
    'fields' => array(
      '#tree' => TRUE,
    ),
  );
  $form['node_fields']['#theme'] = 'node_field_node_field_page_form_fields';
  // Add existing fields form elems.
  _node_field_node_field_page_fields_form($form, $form_state, $node_fields);
  // Add new field form elems.
  _node_field_node_field_page_new_field_form($form, $form_state);
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler on "Save" button.
 */
function node_field_node_field_form_submit($form, &$form_state) {
  // Update existing fields
  if (isset($form_state['values']['node_fields']['fields'])) {
    foreach ($form_state['values']['node_fields']['fields'] as $field) {
      node_field_update_node_field($field);
    }
  }
  // Create new field
  $new_node_field = $form_state['values']['node_fields']['new'];
  if (!empty($new_node_field['title'])) {
    // Add default values.
    $new_node_field += array(
      'nid' => $form_state['node']->nid,
      'show_title' => TRUE,
    );
    if (node_field_create_node_field($new_node_field)) {
      drupal_set_message(t('Field !name successfuly created.', array('!name' => $new_node_field['title'])));
    }
    else {
      drupal_set_message(t('Field !name wasn\'t created.', array('!name' => $new_node_field['title'])), 'error');
    }
  }
}

/**
 * Fields form.
 */
function _node_field_node_field_page_fields_form(&$form, &$form_state, $node_fields) {
  if (empty($node_fields)) {
    return;
  }
  // Building node field form.
  foreach ($node_fields as $key => $node_field) {
    $node_field['key'] = $key;
    $field_form = _node_field_node_field_page_field_form($form_state, $node_field);
    drupal_alter('node_field_field_form', $field_form, $form_state);
    $form['node_fields']['fields'][] = $field_form;
  }
}

function _node_field_node_field_page_field_form(&$form_state, $node_field) {
  // Building node field form.
  $form = array(
    '#type' => 'fieldset',
    '#title' => $node_field['title'],
  );
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $node_field['id'],
  );
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node_field['nid'],
    '#access' => FALSE,
  );    
  $form['title'] = array(
    '#markup' => $node_field['title'],
    '#title' => t('Title'),
    '#title_display' => 'invisible',
  );
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#title_display' => 'invisible',
    '#default_value' => $node_field['weight'],
    '#delta' => ceil(count($form_state['node_fields'])/2),
  );
  $form['type'] = array(
    '#markup' => $node_field['type'],
    '#title' => t('Type'),
    '#title_display' => 'invisible',
  );
  $form['operations'] = array(
    '#type' => 'container',
    '#title' => 'operations',
  );
  $destination = drupal_get_destination();
  $link_attributes = array(
    'query' => array(
      'destination' => $destination['destination'], 
    ),
  );
  $operations = array();
  $operations[] = l(t('edit'), 'node/' . $form['nid']['#value'] . '/node-field/' . $node_field['id'] . '/edit', $link_attributes);
  $operations[] = l(t('delete'), 'node/' . $form['nid']['#value'] . '/node-field/' . $node_field['id'] . '/delete', $link_attributes);
  $form['operations']['op'] = array(
    '#markup' => theme('item_list', array('items' => $operations)),
  );
  return $form;
}

function _node_field_node_field_page_new_field_form(&$form, $form_state) {
  //Building new field form
  $form['node_fields']['new'] = array(
    '#tree' => TRUE,
  );
  $form['node_fields']['new']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Add new field'),
    '#description' => t('Label'),
  );
  $count_field = isset($form['node_fields']['fields']) ? count($form['node_fields']['fields']) : 0;
  $weight_delta = ceil(count($count_field)/2);
  $form['node_fields']['new']['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#title_display' => 'invisible',
    '#default_value' => $weight_delta,
    '#delta' => $weight_delta,
  );
  $form['node_fields']['new']['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#title_display' => 'invisible',
    '#description' => t('Field type'),
    '#options' => node_field_node_field_types_options(),
  );
}

/**
 * Get node field types options.
 *
 * @return
 *   Associative array of options.
 */
function node_field_node_field_types_options() {
  $options = array();
  // Populate node field types options.
  $node_field_types = node_field_types_info();
  foreach ($node_field_types as $node_field_type) {
    $options[$node_field_type['type']] = $node_field_type['label'];
  }
  return $options;
}

/**
 * Delete node_field
 *
 * @param object $node
 * @param array $node_field
 */
function node_field_node_field_delete_form($form, &$form_state, $node, $node_field) {
  $destination = 'node/' . $node->nid . '/node-field';
  $question = t('Are you sure you want to delete node field !title', array('!title' => $node_field['title']));
  $form = confirm_form($form, $question, $destination);
  $form_state['node_field'] = $node_field;
  return $form;
}

function node_field_node_field_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    node_field_delete_node_field($form_state['node_field']);
  }
}

function node_field_node_field_edit_form($form, &$form_state, $node, $node_field) {
  $form_state['node_fields'][] = $node_field;
  $form_state['node'] = $node;
  // Building node field form.
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $node_field['id'],
  );
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node_field['nid'],
    '#access' => FALSE,
  );    
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => $node_field['title'],
    '#length' => 256,
  );
  $form['show_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show title'),
    '#title_display' => 'before',
    '#field_suffix' => t('Yes'),
    '#default_value' => $node_field['show_title'],
  );
  $form['settings'] = node_field_node_field_settings_form($node_field);
  $form['settings']['#tree'] = TRUE;
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler on "Save" button.
 */
function node_field_node_field_edit_form_submit($form, &$form_state) {
  // Update field
  $field = $form_state['values'];
  node_field_update_node_field($field);
}