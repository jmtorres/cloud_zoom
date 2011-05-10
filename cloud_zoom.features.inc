<?php

/**
 * @file cloud_zoom.features.inc
 * Contains the implementions to enable Features Exportable settings
 */


/**
 * Implementation of hook_features_export().
 */
function cloud_zoom_profile_features_export($data, &$export, $module_name) {
  $pipe = array();

  foreach($data as $profile) {
    if ($settings = cloud_zoom_get_settings($profile)) {
      $export['features']['cloud_zoom_preset'][$profile] = $profile;
      $pipe['imagecache'][] = $settings['view_preset'];
      $pipe['imagecache'][] = $settings['zoom_preset'];
    }
  }

  $export['dependencies'] += array(
    'features' => 'features',
    'imagefield' => 'imagefield',
    'imageapi' => 'imageapi',
    'imagecache' => 'imagecache',
    'librares' => 'libraries',
  );

  return $pipe;
}


/**
 * Implementation of hook_features_export_options().
 */
function cloud_zoom_profile_features_export_options() {
  $options = array();
  foreach (cloud_zoom_get_settings() as $profile) {
    $options[$profile['name']] = $profile['name'];
  }
  return $options;
}


/**
 * Implementation of hook_features_export_options_render().
 */
function cloud_zoom_profile_features_export_render($module_name, $data) {
  $items = array();

  foreach ($data as $name) {
    $preset = cloud_zoom_get_settings($name);
    _features_sanitize($preset);
    $items[$name] = $preset;
  }

  $code = '  return '. features_var_export($items, '  ') .";\n";
  return array('cloud_zoom_preset_info' => $code);
}


/**
 * Implementation of hook_features_revert().
 */
function cloud_zoom_profile_features_revert($module) {
  // Get the settings for the Feature module
  $profiles = module_invoke($module, 'cloud_zoom_preset_info');

  // For each profile, if there are any, delete any overriden settings
  if (!empty($profiles)) {
    foreach ($profiles as $profile) {
      db_query("DELETE FROM {cloud_zoom_profiles} WHERE name = '%s'", $profile['name']);
    }
  }

  // Reset the cache
  cloud_zoom_get_settings(NULL, TRUE);
}
