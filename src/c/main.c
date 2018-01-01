#include <pebble.h>

static Window *s_main_window;
static Layer *s_canvas;
static TextLayer *s_time_layer_h;
static TextLayer *s_date_layer;
static int s_hour;
static int s_time_x;
static int s_wind_w;
static int s_wind_h;
//static int s_Times;
static GFont s_time_font_h;
static GFont s_date_font;
GColor ColorBG;
GColor CurrentColor;
GColor TopColor;
GColor BotColor;
GColor SideColor;
GColor ColorToSet;
GColor BaseColor;

#if defined(PBL_COLOR)
void set_bitmap_pixel_color(GBitmap *bitmap, GBitmapFormat bitmap_format, int y, int x, GColor color) {
  GBitmapDataRowInfo row = gbitmap_get_data_row_info(bitmap, y);
  if ((x >= row.min_x) && (x <= row.max_x)) {
    switch(bitmap_format) {
      case GBitmapFormat1Bit :
        row.data[x / 8] ^= (-(gcolor_equal(color, GColorWhite)? 1 : 0) ^ row.data[x / 8]) & (1 << (x % 8));
        break;
      case GBitmapFormat1BitPalette :
        //TODO
        break;
      case GBitmapFormat2BitPalette :
        //TODO
        break;
      case GBitmapFormat4BitPalette :
        //TODO
        break;
      case GBitmapFormat8BitCircular :
      case GBitmapFormat8Bit :
        row.data[x] = color.argb;
    }
  }
}
GColor get_bitmap_color_from_palette_index(GBitmap *bitmap, uint8_t index) {
  GColor *palette = gbitmap_get_palette(bitmap);
  return palette[index];
}
GColor get_bitmap_pixel_color(GBitmap *bitmap, GBitmapFormat bitmap_format, int y, int x) {
  GBitmapDataRowInfo row = gbitmap_get_data_row_info(bitmap, y);
  switch(bitmap_format) {
    case GBitmapFormat1Bit :
      return ((row.data[x / 8] >> (x % 8)) & 1) == 1 ? GColorWhite : GColorBlack;
    case GBitmapFormat1BitPalette :
      return get_bitmap_color_from_palette_index(bitmap, ((row.data[x / 8] << (x % 8)) & 128) == 128 ? 1 : 0);
    case GBitmapFormat2BitPalette :
      return GColorBlack; //TODO get_color_from_palette_index(bitmap, (row.data[x / 4] >> (x % 4)) & 3);
    case GBitmapFormat4BitPalette :
      return GColorBlack; //TODO
    case GBitmapFormat8BitCircular :
    case GBitmapFormat8Bit :
      return (GColor) {.argb = row.data[x] };
  }
  return GColorClear;
}

static GColor color_darken(GColor BaseColor, int s_Times) {
ColorToSet = GColorFromRGB(BaseColor.r*85/s_Times, BaseColor.g*85/s_Times, BaseColor.b*85/s_Times);
return ColorToSet;
}

void layer_update_proc(Layer *layer, GContext *ctx) {
	// Get the framebuffer
	GBitmap *fb = graphics_capture_frame_buffer(ctx);
	GBitmapFormat fb_format = gbitmap_get_format(fb);

// Iterate over all rows
	for(int y = 1; y < s_wind_h; y++) {
   for(int x = 1; x < s_wind_w; x++) {
     GColor CurrentColor = get_bitmap_pixel_color(fb, fb_format, y, x);
     if(gcolor_equal(CurrentColor, GColorWhite)){
       // do nothing 
     } else {
       GColor ColorToSet = ColorBG;
       GColor TopColor = get_bitmap_pixel_color(fb, fb_format, y-1, x);
      
       if(gcolor_equal(TopColor, GColorWhite)){
			 // darken
         GColor ColorToSet = color_darken(CurrentColor,5);
set_bitmap_pixel_color(fb, fb_format, y, x, ColorToSet);
       } else {

       GColor SideColor = get_bitmap_pixel_color(fb, fb_format, y, x-1);
       if(gcolor_equal(SideColor, GColorWhite)){
		   	 // darken
         GColor ColorToSet =color_darken(CurrentColor,3);
set_bitmap_pixel_color(fb, fb_format, y, x, ColorToSet);
       } else {
        	GColor SideColor = get_bitmap_pixel_color(fb, fb_format, y, x+1);
         if(gcolor_equal(SideColor, GColorWhite)){
		   	 // darken
          GColor ColorToSet = color_darken(CurrentColor,3);
set_bitmap_pixel_color(fb, fb_format, y, x, ColorToSet);
         } else {

       
GColor BotColor = get_bitmap_pixel_color(fb, fb_format, y+1, x);
        if(gcolor_equal(BotColor, GColorWhite)){
		   	 // darken
         GColor ColorToSet =color_darken(CurrentColor,2);
set_bitmap_pixel_color(fb, fb_format, y, x, ColorToSet);
       } else {
        set_bitmap_pixel_color(fb, fb_format, y, x, ColorBG);
}
       }

       }



       }   




     }
   }
  }


	for(int y = 0; y < s_wind_h-1; y++) {
   for(int x = 0; x < s_wind_w; x++) {
     GColor CurrentColor = get_bitmap_pixel_color(fb, fb_format, y, x);
     GColor SideColor = get_bitmap_pixel_color(fb, fb_format, y, x+1);
     GColor BotColor = get_bitmap_pixel_color(fb, fb_format, y+1, x);
     GColor TopColor = get_bitmap_pixel_color(fb, fb_format, y+1, x+1);

         // merge
       GColor ColorToSet = GColorFromRGB((CurrentColor.r*3+BotColor.r*2+TopColor.r+SideColor.r*2)*85/8, (CurrentColor.g*3+BotColor.g*2+TopColor.g+SideColor.g*2)*85/8, (CurrentColor.b*3+BotColor.b*2+TopColor.b+SideColor.b*2)*85/8);
set_bitmap_pixel_color(fb, fb_format, y, x, ColorToSet);
   }
  }


graphics_release_frame_buffer(ctx, fb);
}


// Draw shader
static void drawShader(Layer * window_layer){
	// set canvas for shader
		s_canvas = layer_create(GRect(0,0, s_wind_w, s_wind_h));
 		layer_set_update_proc(s_canvas, layer_update_proc);
	  	layer_add_child(window_layer, s_canvas);
}

#endif

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer_hour[4];
  strftime(s_buffer_hour, sizeof(s_buffer_hour),"%H", tick_time);
   s_hour = ((s_buffer_hour[0] - '0')*10)+s_buffer_hour[1] - '0';


static char s_buffer_d[8];
strftime(s_buffer_d, sizeof(s_buffer_d), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
 


  // Display this time on the TextLayer
  if (s_hour == 6) {
    	text_layer_set_text(s_time_layer_h, "almost breakfast");
  	  s_time_x = 2;
	  #if defined(PBL_COLOR)
     ColorBG = GColorBulgarianRose;
	  #endif
  } else if (s_hour == 7) {
    	text_layer_set_text(s_time_layer_h, "breakfast");
	    s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorRed;
	  #endif
  } else if (s_hour == 8) {
    	text_layer_set_text(s_time_layer_h, "almost second breakfast");
	  s_time_x = 3;
	  #if defined(PBL_COLOR)
     ColorBG = GColorOrange;
	  #endif
  } else if (s_hour == 9) {
    	text_layer_set_text(s_time_layer_h, "second breakfast");
	  s_time_x = 2;
	  #if defined(PBL_COLOR)
     ColorBG = GColorKellyGreen;
	  #endif
  } else if (s_hour == 10) {
    	text_layer_set_text(s_time_layer_h, "almost elevenses");
	  s_time_x = 2;
	  #if defined(PBL_COLOR)
     ColorBG = GColorJaegerGreen;
	  #endif
  } else if (s_hour == 11) {
    	text_layer_set_text(s_time_layer_h, "elevenses");
	  s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorDarkGreen;
	  #endif
  } else if (s_hour == 12) {
    	text_layer_set_text(s_time_layer_h, "luncheon");
	  s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorMidnightGreen;
	  #endif
  } else if (s_hour == 13) {
    	text_layer_set_text(s_time_layer_h, "after lunch nap");
	  s_time_x = 2;
	  #if defined(PBL_COLOR)
     ColorBG = GColorOxfordBlue;
	  #endif
  } else if (s_hour == 14) {
    	text_layer_set_text(s_time_layer_h, "afternoon tea");
	  s_time_x = 2;
	  #if defined(PBL_COLOR)
     ColorBG = GColorCadetBlue;
	  #endif
  } else if (s_hour == 15) {
    	text_layer_set_text(s_time_layer_h, "three~ish");
	  s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorCobaltBlue;
	  #endif
  } else if (s_hour == 16) {
    	text_layer_set_text(s_time_layer_h, "almost dinner");
	  s_time_x = 2;
	  #if defined(PBL_COLOR)
     ColorBG = GColorBlue;
	  #endif
  } else if (s_hour == 17) {
    	text_layer_set_text(s_time_layer_h, "dinner");
	  s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorCobaltBlue;
	  #endif
  } else if (s_hour == 18) {
    	text_layer_set_text(s_time_layer_h, "almost supper");
	  s_time_x = 2;
	  #if defined(PBL_COLOR)
     ColorBG = GColorOxfordBlue;
	  #endif
  } else if (s_hour == 19) {
    	text_layer_set_text(s_time_layer_h, "supper");
	  s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorOrange;
	  #endif
  } else if (s_hour == 20) {
    	text_layer_set_text(s_time_layer_h, "eight~ish");
	  s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorRed;
	  #endif
  } else if (s_hour == 21) {
    	text_layer_set_text(s_time_layer_h, "nine~ish");
	  s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorBulgarianRose;
	  #endif
  } else if (s_hour == 24) {
    	text_layer_set_text(s_time_layer_h, "midnight snack");
	  s_time_x = 2;
	  #if defined(PBL_COLOR)
     ColorBG = GColorOxfordBlue;
	  #endif
  } else {
    	text_layer_set_text(s_time_layer_h, "sleep");
	  s_time_x = 1;
	  #if defined(PBL_COLOR)
     ColorBG = GColorBlack;
	  #endif
  }

	text_layer_set_text(s_date_layer, s_buffer_d+(('0' == s_buffer_d[0])?1:0));

 GRect frame =  layer_get_frame(text_layer_get_layer(s_time_layer_h));
 frame.origin.y = ((s_wind_h - (36*s_time_x))/2)-10;
 layer_set_frame(text_layer_get_layer(s_time_layer_h), frame);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_wind_w = bounds.size.w;
  s_wind_h = bounds.size.h;
  
	
  // Create the TextLayer with specific bounds
  s_time_layer_h = text_layer_create(
      GRect(0, 0, bounds.size.w, bounds.size.h));

  s_date_layer = text_layer_create(
      GRect(bounds.size.w/2-30, 0, 60, 22));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer_h, GColorClear);
  text_layer_set_text_color(s_time_layer_h, GColorWhite);
  text_layer_set_text(s_time_layer_h, "dinner?");
  text_layer_set_text_alignment(s_time_layer_h, GTextAlignmentCenter);

text_layer_set_background_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "00:00");
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	
  // Create GFont
  s_time_font_h = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_HOUR_36));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Font_Date_16));

  // Apply to TextLayer
  text_layer_set_font(s_time_layer_h, s_time_font_h);
  text_layer_set_font(s_date_layer, s_date_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer_h));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

		// create shader layer
	#if defined(PBL_COLOR)
	drawShader(window_layer);
   #endif
}

	
static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer_h);
  text_layer_destroy(s_date_layer);

  // Unload GFont
  fonts_unload_custom_font(s_time_font_h);
  fonts_unload_custom_font(s_date_font);

}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();



  // Set the background color
  ColorBG = GColorBlack;
  window_set_background_color(s_main_window, ColorBG);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  

  //prv_update_display();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}