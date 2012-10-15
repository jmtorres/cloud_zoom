(function ($) {
Drupal.behaviors.mybehavior = {
  attach: function (context, settings) {
    // The cloud zoom options are in the HTML.
    $('.cloud-zoom, .cloud-zoom-gallery').CloudZoom();
    $('.cloud-zoom, .cloud-zoom-gallery').parent().css('float', 'left');
  }
};
})(jQuery);
