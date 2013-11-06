(function($) {
  Drupal.behaviors.cloudZoom = {
    attach: function(context, settings) {
      $('.cloudzoom, .cloudzoom-gallery', context).once('cloud-zoom', function(){
        $(this).CloudZoom();
      });
    }
  };
})(jQuery);
