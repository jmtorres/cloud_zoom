<?php

/**
 * @file
 * This file contains al the admin-only function
 */


/**
 * The admin overview page callback - provides a table of fields configured with the cloud_zoom display settings
 */
function cloud_zoom_admin_overview() {
  // Load the common file for the dependency checker
  module_load_include('inc', 'cloud_zoom', 'cloud_zoom.common');

  if ( ($check = cloud_zoom_dependency_check()) && !$check['exists']) {
    drupal_set_message(t(
      'The JQuery Cloud Zoom library is not preset. Please install it into: %path',
      array('%path' => $check['path'])
    ), 'error');
  }

  // Get the settings
  $cloud_zoom_presets = cloud_zoom_get_settings();

  // Get the Imagecache presets
  $imagecache_presets = imagecache_presets();

  // Define an error semaphore - this is used to store multiple errors being outputter
  $error_semaphore = TRUE;
  $error_string = '<span class="error">'. t('Not Set!') .'</span>';

  // Create a row for every preset
  $rows = array();
  foreach ($cloud_zoom_presets as $preset) {
    // Generate the settings list (or the default text)
    if (empty($preset['settings'])) {
      $settings_list = t('Default Settings');
    }
    else {
      $items = array();
      foreach ($preset['settings'] as $k => $v) {
        $items[] = t('!setting_name: !setting_value', array('!setting_name' => $k, '!setting_value' => $v));
      }
      $settings_list = theme('item_list', $items, t('Overrides'));
    }

    if ($error_semaphore) {
      $error_semaphore = isset($preset['view_preset']) && 
                         imagecache_preset_by_name($preset['view_preset']) &&
                         isset($preset['zoom_preset']) &&
                         imagecache_preset_by_name($preset['zoom_preset']);
    }

    // Build the Ops list
    switch ($preset['storage']) {
      default :
      case CLOUD_ZOOM_STORAGE_NORMAL :
        $storage = t('Normal');
        $ops = array(
          l(t('Edit'), 'admin/settings/cloudzoom/'. $preset['name'] .'/edit'),
          l(t('Delete'), 'admin/settings/cloudzoom/'. $preset['name'] .'/delete'),
        );
        break;
      
      case CLOUD_ZOOM_STORAGE_DEFAULT :
        $storage = t('Default');
        $ops = array(
          l(t('Override'), 'admin/settings/cloudzoom/'. $preset['name'] .'/edit'),
        );
        break;

      case CLOUD_ZOOM_STORAGE_OVERRIDE :
        $storage = t('Normal');
        $ops = array(
          l(t('Edit'), 'admin/settings/cloudzoom/'. $preset['name'] .'/edit'),
          l(t('Revert'), 'admin/settings/cloudzoom/'. $preset['name'] .'/revert'),
        );
        break;
    }


    // Create a table row
    $rows[] = array(
      check_plain($preset['name']),
      isset($preset['view_preset']) ? check_plain($preset['view_preset']) : $error_string,
      isset($preset['zoom_preset']) ? check_plain($preset['zoom_preset']) : $error_string,
      $settings_list,
      $storage,
      implode(' | ', $ops),
    );
  }

  if (!$error_semaphore) {
    drupal_set_message(t('There are errors that need addressing below'), 'error');
  }

  // Define the headers
  $headers = array(
    t('Preset'),
    t('View Preset'),
    t('Zoom Preset'),
    t('Settings'),
    t('Storage'),
    t('Ops'),
  );

  // Return a table
  return theme('table', $headers, $rows);
}


/**
 * The edit form callback - provides editable fields for all the "default" options
 */
function cloud_zoom_admin_preset_edit_form(&$form_state, $preset = NULL) {
  // Create a form with the preset key stored
  $form = array();

  $form['preset'] = array(
    '#tree' => TRUE,
  );

  if (isset($preset['name'])) {
    $form['#original_preset'] = $preset;
  }

  $form['preset']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Preset Name'),
    '#description' => t('Enter the name for this preset. Only user lowercase alphanumeric values and hyphens.'),
    '#default_value' => isset($preset['name']) ? $preset['name'] : '',
    '#required' => TRUE,
  );

  if (isset($preset['storage']) && $preset['storage'] == CLOUD_ZOOM_STORAGE_NORMAL) {
    $form['preset']['name']['#type'] = 'item';
    $form['preset']['name']['#value'] = $form['preset']['name']['#default_value'];
  }

  // For every imagecache preset, create a presetname => presetname pair
  $imagecache_presets = array();
  foreach (imagecache_presets() as $id => $imagecache_preset) {
    $imagecache_presets[$imagecache_preset['presetname']] = $imagecache_preset['presetname'];
  }

  // Create a fieldset with options to set the view and zoom presets
  $form['preset']['imagecache_presets'] = array(
    '#type' => 'fieldset',
    '#title' => t('Imagecache Settings'),
    '#description' => t('Set the preview (zoomed out) and zoom Imagecache Presets below'),
  );

  $form['preset']['imagecache_presets']['view_preset'] = array(
    '#title' => t('View Preset'),
    '#description' => t('Set the Imagecache preset to use for the preview (ie the "zoomed out" view)'),
    '#type' => 'select',
    '#options' => $imagecache_presets,
    '#default_value' => isset($preset['view_preset']) ? $preset['view_preset'] : '',
    '#parents' => array('preset', 'view_preset'),
  );

  $form['preset']['imagecache_presets']['zoom_preset'] = array(
    '#title' => t('Zoom Preset'),
    '#description' => t('Set the Imagecache preset to use for the zoomed in view'),
    '#type' => 'select',
    '#options' => $imagecache_presets,
    '#default_value' => isset($preset['zoom_preset']) ? $preset['zoom_preset'] : '',
    '#parents' => array('preset', 'zoom_preset'),
  );


  // Create a settings fieldset with tree enabled
  $form['preset']['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
  );

  // Create a form element for every "default" setting to allow override
  $defaults = _cloud_zoom_default_settings();
  foreach ($defaults as $option => $values) {
    // The default value is either the overridden value or the default provided by _cloud_zoom_default_settings().
    $values['default'] = isset($preset['settings'][$option]) ? $preset['settings'][$option] : $values['default'];

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
    $form['preset']['settings'][$option] = $field_settings + array(
      '#type' => 'textfield',
      '#title' => $option,
      '#description' => $values['description'],
      '#default_value' => $values['default'],
      '#size' => 4,
      '#maxlength' => 4,
    );
  }

  // We need at least a submit, with its own submit handler
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('cloud_zoom_admin_preset_edit_form_submit_save'),
  );

  // TODO - add a reset IF it can be reset
  /*
  $form['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => array('cloud_zoom_admin_preset_edit_form_submit_reset'),
  );
  */

  return $form;
}


/**
 * Validate handler for the above form
 */
function cloud_zoom_admin_preset_edit_form_validate(&$form, &$form_state) {
  // Do not validate the name if it is not set
  if (!isset($form_state['values']['name'])) {
    return;
  }

  // Get the name out for easier reference
  $name = $form_state['values']['name'];

  // If the name contains any non a-z, 0-9 and underscore characters then set an error and return (dont bother checking anything else)
  if (preg_match('/[^a-z0-9_]/', $name)) {
    form_set_error('name', t('Only use lowercase a-z, 0-9 and hyphens.'));
    return;
  }

  // If the name is a valid name AND different to the original preset name,
  // lets check it's not already used
  if (isset($form['#original_preset']) && $form['#original_preset']['name'] != $name) {
    if (cloud_zoom_get_settings($name)) {
      form_set_error('name',
        t('The preset @name already exists', array('@name' => $name))
      );
    }
  }
}


/**
 * Submit handler for the reset button - this removes all the default setting overrides for the current node/field/display options
 */
function cloud_zoom_admin_preset_edit_form_submit_reset(&$form, &$form_state) {
  // TODO

  // Redirect the form back to the overview
  $form_state['redirect'] = 'admin/settings/cloudzoom';
}


/**
 * Submit handler for the save button - this saves any settings which differ from the defaults. If there are no differing options, the sub-section is removed.
 */
function cloud_zoom_admin_preset_edit_form_submit_save(&$form, &$form_state) {
  // Get the preset from the form
  $preset = $form_state['values']['preset'];

  if (isset($form['#original_preset'])) {
    if ($form['#original_preset']['storage'] != CLOUD_ZOOM_STORAGE_NORMAL) {
      $preset['name'] = $form['#original_preset']['name'];
    }

    // Remove the old settings - this stops dupes when you rename
    db_query("DELETE FROM {cloud_zoom_presets} WHERE name = '%s'", $form['#original_preset']['name']);
  }

  // Get the default settings
  $defaults = _cloud_zoom_default_settings();
  dpm($defaults);
  // For every submitted setting, if the value equals the default then unset it
  dpm($preset);
  foreach ($preset['settings'] as $option => $val) {
    if ($val == $defaults[$option]['default']) {
      unset($preset['settings'][$option]);
    }
  }
  dpm($preset);

  // Write changes
  drupal_write_record('cloud_zoom_presets', $preset);

  // Clear the cache
  cloud_zoom_get_settings(NULL, TRUE);

  // Rebuild some other caches - need to do this in case a preset is renamed or new
  // @todo - make a flag to make this 'optional'
  drupal_rebuild_theme_registry();
  content_clear_type_cache();

  // Redirect to the overview
  $form_state['redirect'] = 'admin/settings/cloudzoom';
}


/**
 * Delete preset confirm form
 */
function cloud_zoom_admin_preset_delete_confirm(&$form_state, $preset) {
  // We're ok to continue - create a form and store the preset
  $form = array();
  $form['#original_preset'] = $preset;

  // Return the form through the confirm_form function
  return confirm_form(
    $form,
    t('Are you sure you want to delete the Cloud Zoom Preset %preset?', array('%preset' => $preset['name'])),
    'admin/settings/cloudzoom',
    t('Deleting a preset cannot be undone'),
    t('Delete'),
    t('Cancel'));
}


/**
 * Delete preset confirm form submit hanlder
 */
function cloud_zoom_admin_preset_delete_confirm_submit(&$form, &$form_state) {
  // Get the preset
  $preset = $form['#original_preset'];

  // Delete the preset
  db_query("DELETE FROM {cloud_zoom_presets} WHERE name = '%s'", $preset['name']);

  // Wipe any caches and static storage
  cloud_zoom_get_settings(TRUE);
  drupal_rebuild_theme_registry();
  content_clear_type_cache();

  // Redirect to the admin overview page
  $form_state['redirect'] = 'admin/settings/cloudzoom';
}

// TODO: Revert Confirm Form
