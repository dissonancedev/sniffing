<?php

/**
 * Page callback.
 */
function node_field_node_field_page($node) {
  $content = drupal_get_form('node_field_node_field_page_form', $node);
  return $content;
}

/**
 * Page form.
 */
function node_field_node_field_page_form($form, &$form_state, $node) {
  // Get fields values.
  if (isset($form_state['values']['node_field']['fields'])) {
    $node_fields = $form_state['values']['node_field']['fields'];
  }
  else {
    $node_fields = node_field_db_field_select(array('nid' => $node->nid), array('nid'));
  }
  
  // Populate status values.
  _node_field_node_field_page_populate_status_values($node_fields);
  
  // Detect clicked button.
  $clicked_button = isset($form_state['clicked_button']) ? $form_state['clicked_button'] : FALSE;
  
  // Process clicked buttons.
  _node_field_node_field_page_process_add_field_button($clicked_button, $node_fields);
  _node_field_node_field_page_process_remove_field_button($clicked_button, $node_fields);
  
  // Add vars to $form_state.
  $form_state['node'] = $node;
  $form_state['wrapper_id'] = 'node-field-node-field-form';
  $form_state['node_fields'] = $node_fields;
  
  // Building node fields form.
  $form['node_field'] = _node_field_node_field_page_fields_form($form_state, $node_fields);
  
  // Return form with standard submit button and cancel link.
  $form_title = t('Edit fields for %node_name', array('%node_name' => $form_state['node']->title));
  $form_link = 'node/' . $form_state['node']->nid;
  $form = confirm_form($form, $form_title, $form_link, '', t('Save'), t('Cancel'));
  
  return $form;
}

/**
 * Default validate handler.
 */
function node_field_node_field_page_form_validate($form, &$form_state) {
  // Detect clicked button.
  $clicked_button = isset($form_state['clicked_button']) ? $form_state['clicked_button'] : FALSE;
  
  // Validate clicked buttons.
  _node_field_node_field_page_validate_save_button($clicked_button, $form, &$form_state);
}

/**
 * Default submit handler.
 */
function node_field_node_field_page_form_submit($form, &$form_state) {
  $node_fields = &$form_state['values']['node_field']['fields'];
  
  if (empty($node_fields)) {
    return;
  }
  
  // Save fields changes.
  foreach ($node_fields as &$node_field) {
    if ($node_field['removed']) {
      if (!$node_field['new']) {
        $node_field['op'] = 'delete';
        node_field_db_field_delete($node_field, array('id'));
      }
    }
    else if ($node_field['new']) {
      $node_field['op'] = 'insert';
      node_field_db_field_insert($node_field);
    }
    else {
      $node_field['op'] = 'update';
      if (!empty($node_field['no_need_updating'])) {
        continue;
      } 
      node_field_db_field_update($node_field, array('id'));
    }
  }
}

/**
 * Ajax handler.
 */
function node_field_node_field_page_js($form, $form_state) {
  return $form['node_field'];
}

/**
 * Fields form.
 */
function _node_field_node_field_page_fields_form(&$form_state, $node_fields) {
  // Building node fields form.
  $form = array(
    '#type' => 'container',
    '#attributes' => array('id' => $form_state['wrapper_id']),
    '#tree' => TRUE,
    'fields' => array(
      '#tree' => TRUE,
    ),
  );
  
  // "Add field" button.
  $form['add'] = array(
    '#weight' => 10,
    '#type' => 'button',
    '#name' => 'add',
    '#value' => t('Add field'),
    '#ajax' => array(
      'callback' => 'node_field_node_field_page_js',
      'wrapper' => $form_state['wrapper_id'],
      'method' => 'replace',
      'effect' => 'fade',
    ),
    '#attributes' => array('class' => array('button add')),
    '#node_field_add_field' => TRUE,
  );
  
  // Building node field form.
  foreach ($node_fields as $key => $node_field) {
    $node_field['key'] = $key;
    $form['fields'][] = _node_field_node_field_page_field_form(&$form_state, $node_field);
  }
  
  return $form;
}

function _node_field_node_field_page_field_form(&$form_state, $node_field) {
  // Default node field values.
  $default_node_field = array(
    'title' => '',
    'show_title' => FALSE,
    'value' => '',
  );
  $node_field += $default_node_field;
  
  // Building node field form.
  $form = array(
    '#type' => 'fieldset',
    '#access' => !$node_field['removed'],
    '#title' => $node_field['new'] ? t('New field') : $node_field['title'],
  );
  
  // Field values.
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
    '#field_suffix' => 'Yes',
    '#default_value' => $node_field['show_title'],
  );
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Field type'),
    '#options' => node_field_node_field_types_options(),
    '#empty_option' => t('- Select value -'),
    '#empty_value' => 0,
    '#ajax' => array(
      'callback' => 'node_field_node_field_page_js',
      'wrapper' => $form_state['wrapper_id'],
      'method' => 'replace',
      'effect' => 'fade',
    ),
    '#default_value' => $node_field['type'],
  );
  $form['value'] = node_field_node_field_widget_form($node_field['type']);
  $form['value'] += array(
    '#title' => t('Value'),
    '#default_value' => $node_field['value'],
  );
  
  // Hidden values.
  if (!$node_field['new']) {
    $form['id'] = array(
      '#type' => 'value',
      '#value' => $node_field['id'],
    );
  }
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $form_state['node']->nid,
  );
  
  // Status values.
  $form['new'] = array(
    '#type' => 'value',
    '#value' => $node_field['new'],
  );
  $form['removed'] = array(
    '#type' => 'value',
    '#value' => $node_field['removed'],
  );
  
  // "Remove field" button.
  $form['remove'] = array(
    '#type' => 'button',
    '#name' => 'remove_' . $node_field['key'],
    '#value' => t('Remove item'),
    '#ajax' => array(
      'callback' => 'node_field_node_field_page_js',
      'wrapper' => $form_state['wrapper_id'],
      'method' => 'replace',
      'effect' => 'fade',
    ),
    '#attributes' => array('class' => array('button remove')),
    '#node_field_remove_field' => $node_field['key'],
  );
  
  return $form;
}

/**
 * Populate status values.
 */
function _node_field_node_field_page_populate_status_values(&$node_fields) {
  foreach ($node_fields as &$node_field) {
    $node_field += array(
      'new' => FALSE,
      'removed' => FALSE,
    );
  }
}

/**
 * Process "Add field" button. 
 */
function _node_field_node_field_page_process_add_field_button($clicked_button, &$node_fields) {
  if (!isset($clicked_button['#node_field_add_field'])) {
    return;
  }
  
  $node_fields[] = array(
    'new' => TRUE,
    'removed' => FALSE,
  );
}

/**
 * Process "Remove field" button. 
 */
function _node_field_node_field_page_process_remove_field_button($clicked_button, &$node_fields) {
  if (!isset($clicked_button['#node_field_remove_field'])) {
    return;
  }
  
  $node_fields[$clicked_button['#node_field_remove_field']]['removed'] = TRUE;
}

/**
 * Validate "Save" button. 
 */
function _node_field_node_field_page_validate_save_button($clicked_button, $form, &$form_state) {
  if ($clicked_button['#array_parents'] != array('actions', 'submit')) {
    return;
  }
  
  $node_fields = &$form_state['values']['node_field']['fields'];
  
  if (empty($node_fields)) {
    return;
  }
  
  foreach ($node_fields as $key => $node_field) {
    if (!empty($node_field['no_need_validation'])) {
      continue;
    }
    if ($node_field['removed']) {
      continue;
    }
    if (!$node_field['title']) {
      form_set_error('node_field][fields][' . $key . '][title', t('Enter title for field!'));
    }
    if (!$node_field['type']) {
      form_set_error('node_field][fields][' . $key . '][type', t('Select type of field!'));
    }
    else if (!$node_field['value']) {
      form_set_error('node_field][fields][' . $key . '][value', t('Enter value for field!'));
    }
  }
}

