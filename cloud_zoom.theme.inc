<?php

/**
 * @file
 * cloud_zoom.theme.inc - Contains all theme implementations
 */


/**
 * Theme handler for the cloud_zoom effect
 */
function theme_cloud_zoom_formatter_imagefield($element) {
  // If there is no item, return blank
  if (empty($element['#item'])) {
    return '';
  }

  // Load the common include for the dependency check
  module_load_include('inc', 'cloud_zoom', 'cloud_zoom.common');

  // Get the JQuery Cycle path. Fail/abort if not present
  if ( ($jquery_cloud_zoom_path = cloud_zoom_dependency_check()) && !$jquery_cloud_zoom_path['exists']) {
    drupal_set_message(t(
      'The JQuery Cycle library is not preset. Please install it into: %path',
      array('%path' => $jquery_cloud_zoom_path['path'],)
    ), 'error');
    return '';
  }

  // Add the cloud zoom JS and CSS
  drupal_add_js($jquery_cloud_zoom_path['path'] .'/cloud-zoom.1.0.2.min.js');
  drupal_add_css($jquery_cloud_zoom_path['path'] .'/cloud-zoom.css');

  drupal_add_css(drupal_get_path('module', 'cloud_zoom') .'/cloud_zoom.theme.css');

  // Get the view and zoom presets from the formatter ID.
  list( , , $presetname) = explode('__', $element['#formatter'], 3);

  // Get the settings to see if there are any non-defaults to apply
  $preset = cloud_zoom_get_settings($presetname);

  // Theme the preview image using imagecache
  $small = theme('imagecache', $preset['view_preset'], $element['#item']['filepath'], $element['#item']['data']['alt'], $element['#item']['data']['title']);

  // Buld the Rel for the link, if there are any settings
  $rel = '';
  if (isset($preset['settings'])) {
    $default_settings = _cloud_zoom_default_settings();
    $rel = array();
    foreach ($preset['settings'] as $option => $val) {
      $val = $default_settings[$option]['quoted'] ? "'{$val}'" : $val;
      $rel[] = "{$option}: {$val}";
    }
    $rel = implode(', ', $rel);
  }


  // Return the preview image as a link to the larger image with a cloud-zoom CSS class
  $link_options = array('html' => TRUE, 'attributes' => array('class' => 'cloud-zoom', 'rel' => $rel));
  $output = l($small, imagecache_create_path($preset['zoom_preset'], $element['#item']['filepath']), $link_options);

  // In theory, preset names should only be [a-z0-9\-] however we use check_plain just to be safe
  $preset_clean = check_plain($preset['name']);
  return "<div class=\"cloud-zoom-wrapper cloud-zoom-wrapper-preset-{$preset_clean} clear-block\">\n  {$output}\n</div>";
}


/**
 * Theme handler for the admin settings grid
 */
function theme_cloud_zoom_admin_settings_table($element) {
  $rows = array();

  foreach (element_children($element) as $key) {
    $title = $element[$key]['#title'];
    unset($element[$key]['#title']);

    $rows[] = array(
      $title,
      drupal_render($element[$key]),
    );
  }

  return theme('table', array(), $rows) . drupal_render($element);
}
