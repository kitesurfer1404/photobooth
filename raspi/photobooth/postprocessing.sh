#!/bin/bash

SCRIPT=$(realpath $0)
BASEDIR=$(dirname $SCRIPT)

EXTENSION_JPG="JPG"
EXTENSION_RAW="CR2"

TARGET_DIR_JPG="$BASEDIR/images/jpg"
TARGET_DIR_RAW="$BASEDIR/images/raw"
TARGET_DIR_PROCESSED="$BASEDIR/images/processed"

### Uncomment one of these to enable some kind of filter-effect for the output image

### HIPSTER
#CONVERT_FILTER_FX="-modulate 100,80,100 -gamma 1.7 -brightness-contrast 5 -contrast"

### GOTHAM
#CONVERT_FILTER_FX="-modulate 120,10,100 -fill #222b6d -colorize 25 -gamma 0.7 -contrast"

### LOMO
#CONVERT_FILTER_FX="-channel R -level 10% -channel G -level 10%"

### 80s PHOTOGRAPH
#CONVERT_FILTER_FX="-set colorspace RGB ( -clone 0 -fill #222b6d -colorize 100% ) ( -clone 0 -colorspace gray -negate ) -compose blend -define compose:args=35,65 -composite ( -clone 0 -fill #f7daae -colorize 100% ) ( -clone 0 -colorspace gray ) -compose blend -define compose:args=35,65 -composite -contrast -modulate 100,150,100 -auto-gamma"

### POLAROID
#CONVERT_FILTER_FX="-set colorspace RGB ( -clone 0 -fill #330000 -colorize 100% ) ( -clone 0 -colorspace gray -negate ) -compose blend -define compose:args=35,65 -composite -modulate 130,100,100 -gamma 1.2 -contrast -contrast"


### Set the output size of the image. This might be slightly larger than
### the viewing size for slide-effects etc.
### Please refer to your slideshow documentation.
CONVERT_RESIZE_OPTIONS="-resize 2400x1600"

### You can also crop a part of the image.
### This is usefull to crop out unwanted background
### and/or crop to a specific format
CONVERT_CROP_OPTIONS="-crop 1800x1200+300+400"

### Store actual image count here for ajax requests.
IMAGE_COUNT_FILE="$BASEDIR/image_count.txt"

function process_jpg {
  #echo "Processing $1"

  tmp=$(find "$TARGET_DIR_PROCESSED" -type f | wc -l)
  #echo "Processed images: $tmp"

  imgcount=$((tmp+1))
  output_filename="$TARGET_DIR_PROCESSED/$imgcount.$EXTENSION_JPG"
  #echo "Output filename: $output_filename"

  convert "${1}" $CONVERT_FILTER_FX $CONVERT_RESIZE_OPTIONS $CONVERT_CROP_OPTIONS "$output_filename"

  #echo "Updating image count file"
  echo "$imgcount" > "$IMAGE_COUNT_FILE"
}


#echo "gphoto ACTION=$ACTION"

if [[ $ACTION == "download" ]]; then

  ### Process all the JPG-files, if there are any.
  ### For each JPG the function "process_jpg" is called
  ### before moving them to the destination directory.
  files=$(ls $BASEDIR/*.$EXTENSION_JPG 2> /dev/null | wc -l)
  if [ "$files" -gt 0 ]
  then
    #echo "$files new .$EXTENSION_JPG"
    
    for f in $BASEDIR/*.$EXTENSION_JPG
    do
      process_jpg "$f"
      #echo "moving "$f" to $TARGET_DIR_JPG" 
      mv "$f" "$TARGET_DIR_JPG"
    done
  fi

  ### Process all RAW-files, if there are any.
  ### This is just moving them to a directory.
  files=$(ls $BASEDIR/*.$EXTENSION_RAW 2> /dev/null | wc -l)
  if [ "$files" -gt 0 ]
  then
    #echo "$files new .$EXTENSION_RAW"
    #echo "moving *.$EXTENSION_RAW to $TARGET_DIR_RAW" 
    mv *.$EXTENSION_RAW "$TARGET_DIR_RAW"
  fi

  #echo "done."
fi
