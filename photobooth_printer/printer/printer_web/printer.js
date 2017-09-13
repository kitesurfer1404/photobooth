function load_new_images() {
  $.ajax({
    type: "GET",
    dataType: "json",
    url: ajax_url,
    cache: false,
    success: function(data){
      if(Number.isInteger(data)) {
        if(data > reference_id) {
          for(i = reference_id+1; i <= data; i++) {
            var image = photobooth_server + image_path + i + image_extension;
            $('#imagelist').append('<img src="' + image + '" alt="photobooth_image" />');
          }
          if(reference_id == 0) {
			$('#imagelist img').first().addClass('current');
			update_current_image();
			scroll_imagelist();
		  }
          reference_id = data;
        }
      }
    }
  });
  ajax_timer = setTimeout(load_new_images,ajax_interval*1000);
}

function scroll_imagelist() {
  var container = $('#imagelist');
  var target = $('#imagelist img.current');
  var x = container.width();
  var y = target.outerWidth(true);
  var z = target.index();
  var q = 0;
  var m = container.find('img');
  for(var i = 0; i < z; i++){
    q += $(m[i]).outerWidth(true);
  }
  container.clearQueue();
  container.stop();
  container.animate({scrollLeft: Math.max(0, q - (x - y)/2)}, 250);
}

function update_current_image() {
    var new_src = $('#imagelist img.current').attr('src');
	$("#current_image").clearQueue();
    $("#current_image").stop();
	$("#current_image").fadeOut(200, function() {;
	    $("#current_image").attr("src", new_src);
    	$("#current_image").fadeIn(500);
	});
}

function setup_keyboard() {
	$("body").keypress(function( event ) {
      switch(event.key) {
		case 'a':
            event.preventDefault();
			var item = $('#imagelist img.current');
		    if(item.prev().length > 0){
                item.removeClass('current');
				item.prev().addClass('current');
				scroll_imagelist();
				update_current_image();
            }
			break;
		case 'b':
            event.preventDefault();
			var item = $('#imagelist img.current');
		    if(item.next().length > 0){
                item.removeClass('current');
				item.next().addClass('current');
				scroll_imagelist();
				update_current_image();
            }
			break;
		case 'c':
            event.preventDefault();
			var item = $('#imagelist img.current');
		    if(item.next().length > 0){
                item.removeClass('current');
				$('#imagelist img').last().addClass('current');
				scroll_imagelist();
				update_current_image();
            }
			break;
		case 'd':
            event.preventDefault();
            var src = $('#imagelist img.current').attr('src');
            var img = src.substr(src.lastIndexOf('/') + 1);
            var anticache = (new Date()).getTime();
			$("#print_image").attr("src", '/printimage/' + anticache + '/' + img);
			break;
        default:
			break;
      }
	});
}

$(document).ready(function(){
  load_new_images();
  setup_keyboard();
});

$.ajaxSetup ({
    cache: false
});
