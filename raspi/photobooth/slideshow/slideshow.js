function get_next_image() {
  if(img_queue.length > 0) {
    var first = img_queue[0];
    img_queue.shift();
    images.push(first);
    //console.log("adding image from queue: " + first);
    return first;
  } else {
    var random = images[Math.floor(Math.random()*images.length)];
    //console.log("adding random image: " + random);
    return random;
  }
}

function kenburns() {
  if(kenburns_index >= kenburns_queue_length) { kenburns_index = 0; }

  //console.log(kenburns_images[kenburns_index]);
  $(kenburns_images[kenburns_index]).addClass('fx');

  // we can't remove the class from the previous element or we'd get a bouncing effect so we clean up the one before last
  // (there must be a smarter way to do this though)
  if(kenburns_index===0){
    //console.log("index 0");
    $(kenburns_images[kenburns_queue_length-2]).removeClass('fx');
    $(kenburns_images[kenburns_queue_length-2]).attr('src', get_next_image());
  }
  if(kenburns_index===1){
    //console.log("index 1");
    $(kenburns_images[kenburns_queue_length-1]).removeClass('fx');
    $(kenburns_images[kenburns_queue_length-1]).attr('src', get_next_image());
  }
  if(kenburns_index>1){
    //console.log("index: " + kenburns_index);
    $(kenburns_images[kenburns_index-2]).removeClass('fx');
    $(kenburns_images[kenburns_index-2]).attr('src', get_next_image());
  }

  kenburns_index++;
  kenburns_timer = setTimeout(kenburns,kenburns_interval*1000);
}

function start_kenburns(target) { 
  $(target + ' img').first().addClass('fx');
  
  kenburns_images = $(target + ' img');
  kenburns_index = 1;
  kenburns_queue_length = kenburns_images.length;
  
  $(target).append('<div id="imagecache" style="display:none; width:0; height:0;"></div>');

  kenburns_timer = setTimeout(kenburns,kenburns_interval*1000);
  //console.log("kenburns started");
}

function load_new_images() {
  $.ajax({
    type: "GET",
    dataType: "json",
    url: ajax_url,
    cache: false,
    success: function(data){
      //console.log("response from server: " + data);
      if(Number.isInteger(data)) {
        if(data > reference_id) {
          for(i = reference_id+1; i <= data; i++) {
            var image = image_path + i + image_extension;
            //console.log("enqueueing image " + image);

            // When the slideshow is loaded the first time, images are pushed
            // to the image list (from where they are loaded in a random order)
            // instead of enqueing them.
            // This way you don't have to watch ALL images from the beginning.
            if(reference_id == 0) { 
              images.push(image);
            } else {
              img_queue.push(image);
            }
            $('#imagecache').append('<img src="' + image + '" alt="' + data + image_extension + '" />');
          }
          reference_id = data;
          //console.log("new reference_id: " + reference_id);
        }
      }
    }
  });
  ajax_timer = setTimeout(load_new_images,ajax_interval*1000);
}

$(document).ready(function(){
  start_kenburns('#slideshow');
  load_new_images();
});

$.ajaxSetup ({
    cache: false
});
