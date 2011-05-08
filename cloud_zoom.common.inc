<?php

/**
 * @file cloud_zoom.common.inc
 * This file contains functions common to the module and install file
 */

/**
 * Dependency checker. This defines the JQuery Cycle path and a value to
 * determine whether or not it is present.
 */
function cloud_zoom_dependency_check() {
  static $result = NULL;

  if (isset($result)) {
    return $result;
  }

  // Ensure Libraries module file is present. There is a risk, without this,
  // that if libraries is being installed 'parallel' to Cloud Zoom then
  // Drupal may not have included the .module file yet
  module_load_include('module', 'libraries');
  $path = module_invoke('libraries', 'get_path', 'cloud_zoom');
  $filepath = $path . '/cloud-zoom.1.0.2.min.js';

  $result = array(
    'path' => $path,
    'filepath' => $filepath,
    'exists' => (is_dir($path) && is_file($filepath)),
  );

  return $result;
}


/**
 * Return the version of JQuery Cycle that is installed
 */
function cloud_zoom_get_jquery_cycle_version($filepath) {
  $version = 0;
  $pattern = '#// Cloud Zoom V([0-9\.]+)#';

  $js_contents = file_get_contents($filepath);
  if (preg_match($pattern, $js_contents, $matches)) {
    $version = $matches[1];
  }

  return $version;
}
