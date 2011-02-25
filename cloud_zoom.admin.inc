<?php

/**
 * @file
 * This file contains al the admin-only function
 */


/**
 * The admin overview page callback - provides a table of fields configured with the cloud_zoom display settings
 */
function cloud_zoom_admin_overview() {
  // Get the settings
  $settings = cloud_zoom_get_settings();

  // Get the Imagecache presets
  $imagecache_presets = imagecache_presets();

  // Define an error semaphore - this is used to store multiple errors being outputter
  $error_semaphore = TRUE;

  // Create a row for every preset
  $rows = array();
  foreach ($settings as $key => $values) {
    // Generate the settings list (or the default text)
    if (empty($values['settings'])) {
      $settings_list = t('Default Settings');
    }
    else {
      $items = array();
      foreach ($values['settings'] as $k => $v) {
        $items[] = t('!setting_name: !setting_value', array('!setting_name' => $k, '!setting_value' => $v));
      }
      $settings_list = theme('item_list', $items, t('Overrides'));
    }

    // Get the view and preset values - if not set, display an error
    $view_preset_val = isset($values['view_preset']) ? check_plain($values['view_preset']) : '<span class="error">Not set!</span>';
    $zoom_preset_val = isset($values['zoom_preset']) ? check_plain($values['zoom_preset']) : '<span class="error">Not set!</span>';

    if ($error_semaphore) {
      // If the preset is not set or, if it is, if there is no preset by that name
      if ((!isset($values['view_preset']) || !imagecache_preset_by_name($values['view_preset'])) ||
          (!isset($values['zoom_preset']) || !imagecache_preset_by_name($values['zoom_preset']))) {
        drupal_set_message(t('There are errors that need addressing below'), 'error');
        $error_semaphore = FALSE;
      }
    }

    // Build the Ops list
    if (isset($values['locked']) && $values['locked']) {
      $ops = array(t('Locked'));
    }
    else {
      $ops = array(
        l(t('Edit'), 'admin/settings/cloudzoom/edit/'. $key),
        l(t('Delete'), 'admin/settings/cloudzoom/delete/'. $key),
      );
    }

    // Create a table row
    $rows[] = array(
      check_plain($key),
      $view_preset_val,
      $zoom_preset_val,
      $settings_list,
      implode(' | ', $ops),
    );
  }

  // Define the headers
  $headers = array(
    t('Preset'),
    t('View Preset'),
    t('Zoom Preset'),
    t('Settings'),
    t('Ops'),
  );

  // Return a table
  return theme('table', $headers, $rows);
}


/**
 * The edit form callback - provides editable fields for all the "default" options
 */
function cloud_zoom_admin_preset_edit_form($form_state, $key) {
  // Get the settings
  $settings = cloud_zoom_get_settings();

  // If there are no settings for this key, create a temporary array
  if (!isset($settings[$key])) {
    $settings[$key] = array();
  }

  // Create a form with the preset key stored
  $form = array();
  $form['key'] = array('#type' => 'value', '#value' => $key);

  // For every imagecache preset, create a presetname => presetname pair
  $imagecache_presets = array();
  foreach (imagecache_presets() as $id => $imagecache_preset) {
    $imagecache_presets[$imagecache_preset['presetname']] = $imagecache_preset['presetname'];
  }

  // Create a fieldset with options to set the view and zoom presets
  $form['imagecache_presets'] = array(
    '#type' => 'fieldset',
    '#title' => t('Imagecache Settings'),
    '#description' => t('Set the preview (zoomed out) and zoom Imagecache Presets below'),
  );

  $form['imagecache_presets']['view_preset'] = array(
    '#title' => t('View Preset'),
    '#description' => t('Set the Imagecache preset to use for the preview (ie the "zoomed out" view)'),
    '#type' => 'select',
    '#options' => $imagecache_presets,
    '#default_value' => isset($settings[$key]['view_preset']) ? $settings[$key]['view_preset'] : '',
  );

  $form['imagecache_presets']['zoom_preset'] = array(
    '#title' => t('Zoom Preset'),
    '#description' => t('Set the Imagecache preset to use for the zoomed in view'),
    '#type' => 'select',
    '#options' => $imagecache_presets,
    '#default_value' => isset($settings[$key]['zoom_preset']) ? $settings[$key]['zoom_preset'] : '',
  );


  // Create a settings fieldset with tree enabled
  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#tree' => TRUE,
  );

  // Create a form element for every "default" setting to allow override
  $defaults = _cloud_zoom_default_settings();
  foreach ($defaults as $option => $values) {
    // The default value is either the overridden value or the default provided by _cloud_zoom_default_settings().
    $values['default'] = isset($settings[$key]['settings'][$option]) ? $settings[$key]['settings'][$option] : $values['default'];

    // Define the field settings based on the option - some are text, some are selects (for example)
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

    // Create the option element based on the settings above
    $form['settings'][$option] = $field_settings + array(
      '#type' => 'textfield',
      '#title' => $option,
      '#description' => $values['description'],
      '#default_value' => $values['default'],
      '#size' => 4,
      '#maxlength' => 4,
    );
  }

  // We need a submit and reset button - each with their own submit handlers
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('cloud_zoom_admin_preset_edit_form_submit_save'),
  );

  $form['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => array('cloud_zoom_admin_preset_edit_form_submit_reset'),
  );

  return $form;
}


/**
 * Validate handler for the above form
 */
function cloud_zoom_admin_preset_edit_form_validate($form, &$form_state) {
  // TODO: Validate
}


/**
 * Submit handler for the reset button - this removes all the default setting overrides for the current node/field/display options
 */
function cloud_zoom_admin_preset_edit_form_submit_reset($form, &$form_state) {
  // Get the database settings
  $settings = variable_get('cloud_zoom_settings', array());

  // Set the settings to an empty array
  $settings[$form_state['values']['key']]['settings'] = array();

  // Save the settings
  variable_set('cloud_zoom_settings', $settings);

  // Redirect the form back to the overview
  $form_state['redirect'] = 'admin/settings/cloudzoom';
}


/**
 * Submit handler for the save button - this saves any settings which differ from the defaults. If there are no differing options, the sub-section is removed.
 */
function cloud_zoom_admin_preset_edit_form_submit_save($form, &$form_state) {
  // Get the default settings
  $defaults = _cloud_zoom_default_settings();

  // Get the stored settings
  $settings = variable_get('cloud_zoom_settings', array());

  // Get the key out for easier reference
  $key = $form_state['values']['key'];

  // Clear the settings for this key (preset)
  $settings[$key]['settings'] = array();

  // For every submitted setting, if the value differs from the default then store it
  foreach ($form_state['values']['settings'] as $option => $val) {
    if ($val != $defaults[$option]['default']) {
      $settings[$key]['settings'][$option] = $val;
    }
  }

  // Store the view and zoom presets
  $settings[$key]['view_preset'] = $form_state['values']['view_preset'];
  $settings[$key]['zoom_preset'] = $form_state['values']['zoom_preset'];

  // Save settings
  variable_set('cloud_zoom_settings', $settings);

  // Redirect to the overview
  $form_state['redirect'] = 'admin/settings/cloudzoom';
}


/**
 * Add preset form
 */
function cloud_zoom_admin_preset_add_form(&$form_state) {
  // This form only needs to be a simple one - a name textfield and a submit button
  $form = array();

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Preset Name'),
    '#required' => TRUE,
    '#description' => t('The preset name is used internally to identify the settings. Lowercase a-z, numbers and underscores only'),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Add'));
  return $form;
}


/**
 * A validate handler for the above form
 */
function cloud_zoom_admin_preset_add_form_validate($form_id, &$form_state) {
  // Gett the name out for easier reference
  $name = $form_state['values']['name'];

  // If the name contains any non a-z, 0-9 and underscore characters then set an error and return (dont bother checking anything else)
  if (preg_match('/[^a-z0-9_]/', $name)) {
    form_set_error('name', t('Only use lowercase a-z, 0-9 and underscores.'));
    return;
  }

  // If the name is a valid name, lets check it's not already used
  $settings = variable_get('cloud_zoom_settings', array());
  if (isset($settings[$name]) && is_array($settings[$name])) {
    form_set_error('name', t('A preset already exists by this name'));
  }
}


/**
 * Submit handler for the above form
 */
function cloud_zoom_admin_preset_add_form_submit($form_id, &$form_state) {
  // Get the settings
  $settings = variable_get('cloud_zoom_settings', array());

  // Pull the name out for easier reference, create a settings entry with empty settings overrides and store
  $name = $form_state['values']['name'];
  $settings[$name] = array('settings' => array());
  variable_set('cloud_zoom_settings', $settings);

  // Rebuild any relevant caches
  cloud_zoom_get_settings(TRUE);
  drupal_rebuild_theme_registry();
  content_clear_type_cache();

  // Redirect to the edit page for this new preset
  $form_state['redirect'] = 'admin/settings/cloudzoom/edit/'. $name;
}


/**
 * Delete preset confirm form
 */
function cloud_zoom_admin_preset_delete_confirm(&$form_state, $key) {
  // Get the settings
  $settings = cloud_zoom_get_settings();

  // If the key isn't set, return a 404
  if (!isset($settings[$key])) {
    drupal_not_found();
    exit;
  }
  // If the setting preset is set, and there is a locked setting which is also set, return a 403
  elseif (isset($settings[$key]['locked']) && $settings[$key]['locked']) {
    drupal_access_denied();
    exit;
  }

  // We're ok to continue - create a form and store the key (preset name)
  $form = array(
    'key' => array('#type' => 'value', '#value' => $key)
  );

  // Return the form through the confirm_form function
  return confirm_form(
    $form,
    t('Are you sure you want to delete the Cloud Zoom Preset %preset?', array('%preset' => $key)),
    'admin/settings/cloudzoom',
    t('Deleting a preset cannot be undone'),
    t('Delete'),
    t('Cancel'));
}


/**
 * Delete preset confirm form submit hanlder
 */
function cloud_zoom_admin_preset_delete_confirm_submit($form_id, &$form_state) {
  // Get the settings
  $settings = variable_get('cloud_zoom_settings', array());

  // Unset the settings for the key provided
  unset($settings[$form_state['values']['key']]);

  // Save the settings
  variable_set('cloud_zoom_settings', $settings);

  // Wipe any caches and static storage
  cloud_zoom_get_settings(TRUE);
  drupal_rebuild_theme_registry();
  content_clear_type_cache();

  // Redirect to the admin overview page
  $form_state['redirect'] = 'admin/settings/cloudzoom';
}
