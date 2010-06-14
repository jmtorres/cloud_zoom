<?php
// $Id$

/**
 * @file
 * This file contains al the admin-only function
 */


/**
 * The admin overview page callback - provides a table of fields configured with the cloud_zoom display
 */
function cloud_zoom_field_admin() {
  $settings = variable_get('cloud_zoom_field_settings', array());

  $output = '';
  $fields = content_fields();
  $node_types = node_get_types();

  $rows = array();
  foreach ($fields as $field) {
    foreach ($field['display_settings'] as $display_name => $display_settings) {
      if (empty($display_settings)) continue;

      if (drupal_substr($display_settings['format'], 0, 24) == 'imagefield__cloud_zoom__') {
        $key = implode('-', array('node', $field['type_name'], $field['field_name'], $display_name));
        $ops = l(t('Edit'), 'admin/settings/cloudzoom/'. $key .'/edit');

        if (isset($settings[$key])) {
          $items = array();
          foreach ($settings[$key] as $k => $v) {
            $items[] = t('!setting_name: !setting_value', array('!setting_name' => $k, '!setting_value' => $v));
          }
          $settings_list = theme('item_list', $items, t('Overrides'));
        }
        else {
          $settings_list = t('Default Settings');
        }

        $rows[] = array(
          t('Node'),
          $node_types[$field['type_name']]->name,
          $display_name,
          $field['field_name'],
          $settings_list,
          $ops,
        );
      }
    }
  }

  if (!empty($rows)) {
    $headers = array(
      t('Type'),
      t('ID'),
      t('Display Type'),
      t('Field'),
      t('Settings'),
      t('Ops'),
    );

    $output .= theme('table', $headers, $rows);
  }

  return $output;
}


/**
 * The edit form callback - provides editable fields for all the "default" options
 */
function cloud_zoom_field_admin_edit_form($form_state, $key) {
  $settings = variable_get('cloud_zoom_field_settings', array());

  if (!isset($settings[$key])) {
    $settings[$key] = array();
  }

  $form = array();
  $form['key'] = array('#type' => 'value', '#value' => $key);

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#tree' => TRUE,
  );

  $defaults = _cloud_zoom_field_default_settings();
  foreach ($defaults as $option => $values) {
    $values['default'] = isset($settings[$key][$option]) ? $settings[$key][$option] : $values['default'];

    $field_settings = array();
    $field_prefix = $options = $type = $size = $maxlength = NULL;

    switch ($option) {
      case 'position' :
        $field_settings['#size'] = 8;
        $field_settings['#maxlength'] = 48;
        break;

      case 'smoothMove' :
        $field_settings['#size'] = $field_settings['#maxlength'] = 1;
        break;

      case 'adjustX' :
      case 'adjustY' :
        $field_settings['#field_suffix'] = 'px';
        break;

      case 'tint' :
        $field_settings['#field_prefix'] = '#';
        $field_settings['#size'] = $field_settings['#maxlength'] = 6;
        break;

      case 'softFocus' :
      case 'showTitle' :
        $field_settings['#options'] = array(0 => 'false', 1 => 'true');
        $field_settings['#type'] = 'select';
        $field_settings['#size'] = 1;
        break;
    }

    $form['settings'][$option] = $field_settings + array(
      '#type' => 'textfield',
      '#title' => $option,
      '#description' => $values['description'],
      '#default_value' => $values['default'],
      '#size' => 4,
      '#maxlength' => 4,
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('cloud_zoom_field_admin_edit_form_submit_save'),
  );

  $form['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => array('cloud_zoom_field_admin_edit_form_submit_reset'),
  );

  return $form;
}


/**
 * Validate handler for the above form
 */
function cloud_zoom_field_admin_edit_form_validate($form, &$form_state) {
  // TODO: Validate
}


/**
 * Submit handler for the reset button - this removes all the default setting overrides for the current node/field/display options
 */
function cloud_zoom_field_admin_edit_form_submit_reset($form, &$form_state) {
  $settings = variable_get('cloud_zoom_field_settings', array());

  $key = $form_state['values']['key'];

  if (isset($settings[$key])) {
    unset($settings[$key]);
  }

  if (empty($settings)) {
    variable_del('cloud_zoom_field_settings');
  }
  else {
    variable_set('cloud_zoom_field_settings', $settings);
  }

  $form_state['redirect'] = 'admin/settings/cloudzoom';
}


/**
 * Submit handler for the save button - this saves any settings which differ from the defaults. If there are no differing options, the sub-section is removed.
 */
function cloud_zoom_field_admin_edit_form_submit_save($form, &$form_state) {
  $defaults = _cloud_zoom_field_default_settings();

  $settings = variable_get('cloud_zoom_field_settings', array());

  $key = $form_state['values']['key'];

  $settings[$key] = array();

  foreach ($form_state['values']['settings'] as $option => $val) {
    if ($val != $defaults[$option]['default']) {
      $settings[$key][$option] = $val;
    }
  }

  if (empty($settings[$key])) {
    unset($settings[$key]);
  }

  if (empty($settings)) {
    variable_del('cloud_zoom_field_settings');
  }
  else {
    variable_set('cloud_zoom_field_settings', $settings);
  }

  $form_state['redirect'] = 'admin/settings/cloudzoom';
}
