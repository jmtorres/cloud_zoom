(function($) {
  Drupal.behaviors.cloudZoom = {
    attach: function(context, settings) {
      $('.cloud-zoom', context).once('cloud-zoom', function(){
        $(this).CloudZoom({position: 'inside'});
      });
    }
  };
})(jQuery);
