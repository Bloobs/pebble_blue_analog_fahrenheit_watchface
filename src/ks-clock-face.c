#include <pebble.h>

#define COLORS       true
#define ANTIALIASING true

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_CITY 2
#define KEY_ICON 3

	
#define HAND_MARGIN  10
#define HAND_MARGIN_SECONDS  2
#define FINAL_RADIUS 65

#define ANIMATION_DURATION 500
#define ANIMATION_DELAY    600

typedef struct {
  int hours;
  int minutes;
	int seconds;
} Time;

static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN, //0
  RESOURCE_ID_IMAGE_CLOUD, //1
  RESOURCE_ID_IMAGE_RAIN, //2
  RESOURCE_ID_IMAGE_SNOW //3
};

static Layer  *s_date_layer;
static TextLayer *s_day_label, *s_num_label, *s_weather_label, *s_city_label;
static char s_num_buffer[4], s_day_buffer[6];

static GPoint Horas[48];
static GPoint Horas_corta[48];
static GPoint Rabo_Horas[48];

static GPoint Minutos[60];
static GPoint Minutos_corto[60];
static GPoint Rabo_Minutos[60]; 

static GPoint Segundos[60];
static GPoint Rabo_Segundos[60];

static Window *s_main_window;
static Layer *s_canvas_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static GPoint s_center;
static GPoint s_center_chrono1;
static GPoint s_center_chrono2;

static Time s_last_time, s_anim_time;
static int avance_hora = 0;
static int s_radius = 0, s_color_channels[3];
static bool s_animating = false;

char* floatToString(char* buffer, int bufferSize, double number)
{
char decimalBuffer[5];
snprintf(buffer, bufferSize, "%d", (int)number);
strcat(buffer, ".");
snprintf(decimalBuffer, 5, "%02d", (int)((double)(number - (int)number) * (double)100));
strcat(buffer, decimalBuffer);
return buffer;
}

static void Inicia_Arrays_Pixeles() {
  // Inicia arrays de pixeles
	static int centro_x = 72;
	static int centro_y = 72;
	static int long_sec = 70;
	static int long_min = 60;
	static int long_hour = 40;
	static int long_min_corto = 45;
	static int long_hour_corta = 25;
	static int long_back_sec = 20;
	static int long_back_min = 12;
	static int long_back_hour = 12;
	static float angulo = 0;
	static float angulo2 = 0;
	//static char strangulo[32];
	
  for(int i = 0; i < 48; i++) {
		angulo = (float) TRIG_MAX_ANGLE * i / 48;
		//angulo = (float) TRIG_MAX_ANGLE * (i % 12) / 12;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Angulos de Hora %d: %d",  i, (int)angulo);

    Horas[i].x = (int16_t)(sin_lookup(angulo) * (int32_t)long_hour / TRIG_MAX_RATIO) + centro_x;
    Horas[i].y = (int16_t)(-cos_lookup(angulo) * (int32_t)long_hour / TRIG_MAX_RATIO) + centro_y;
    Horas_corta[i].x = (int16_t)(sin_lookup(angulo) * (int32_t)long_hour_corta / TRIG_MAX_RATIO) + centro_x;
    Horas_corta[i].y = (int16_t)(-cos_lookup(angulo) * (int32_t)long_hour_corta / TRIG_MAX_RATIO) + centro_y;
		
		if (i >= 24) {
		  //angulo2 = (float) TRIG_MAX_ANGLE * ((i-6) % 12)/12;
		  angulo2 = (float) TRIG_MAX_ANGLE * (i - 24) / 48;
		} else {
		  //angulo2 = (float) TRIG_MAX_ANGLE * ((i+6) % 12)/12;
		  angulo2 = (float) TRIG_MAX_ANGLE * (i + 24) /48;
		}		
		Rabo_Horas[i].x = (int16_t)(sin_lookup(angulo2) * (int32_t)long_back_hour / TRIG_MAX_RATIO) + centro_x;
    Rabo_Horas[i].y = (int16_t)(-cos_lookup(angulo2) * (int32_t)long_back_hour / TRIG_MAX_RATIO) + centro_y;		   			
	}

	for(int j = 0; j < 60; j++) {
		angulo = (float) TRIG_MAX_ANGLE * j / 60;
    Segundos[j].x = (int16_t)(sin_lookup(angulo) * (int32_t)long_sec / TRIG_MAX_RATIO) + centro_x;
    Segundos[j].y = (int16_t)(-cos_lookup(angulo) * (int32_t)long_sec / TRIG_MAX_RATIO) + centro_y;		

		if (j >= 30) {
		  angulo2 = (float) TRIG_MAX_ANGLE * (j - 30) / 60;
		} else {
		  angulo2 = (float) TRIG_MAX_ANGLE * (j + 30) / 60;
		}		
		Rabo_Segundos[j].x = (int16_t)(sin_lookup(angulo2) * (int32_t)long_back_sec / TRIG_MAX_RATIO) + centro_x;
    Rabo_Segundos[j].y = (int16_t)(-cos_lookup(angulo2) * (int32_t)long_back_sec / TRIG_MAX_RATIO) + centro_y;
	}

	for(int k = 0; k < 60; k++) {
		angulo = (float) TRIG_MAX_ANGLE * k / 60;
    Minutos[k].x = (int16_t)(sin_lookup(angulo) * (int32_t)long_min / TRIG_MAX_RATIO) + centro_x;
    Minutos[k].y = (int16_t)(-cos_lookup(angulo) * (int32_t)long_min / TRIG_MAX_RATIO) + centro_y;
    Minutos_corto[k].x = (int16_t)(sin_lookup(angulo) * (int32_t)long_min_corto / TRIG_MAX_RATIO) + centro_x;
    Minutos_corto[k].y = (int16_t)(-cos_lookup(angulo) * (int32_t)long_min_corto / TRIG_MAX_RATIO) + centro_y;

		if (k >= 30) {
		  angulo2 = (float) TRIG_MAX_ANGLE * (k-30) / 60;
		} else {
		  angulo2 = (float) TRIG_MAX_ANGLE * (k+30) / 60;
		}		
		Rabo_Minutos[k].x = (int16_t)(sin_lookup(angulo2) * (int32_t)long_back_min / TRIG_MAX_RATIO) + centro_x;
    Rabo_Minutos[k].y = (int16_t)(-cos_lookup(angulo2) * (int32_t)long_back_min / TRIG_MAX_RATIO) + centro_y;		   				
	}
	
}


/*************************** AnimationImplementation **************************/

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if(handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = animation_started,
      .stopped = animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

/************************************ UI **************************************/

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
	s_last_time.seconds = tick_time->tm_sec;
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours >= 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;
	
	 if (s_last_time.minutes < 15) {
		 avance_hora = s_last_time.hours * 4;
	 }
   else if (s_last_time.minutes < 30) {
		 		 avance_hora = (s_last_time.hours * 4)+1;
         }
   else if (s_last_time.minutes < 45) {
		 		 avance_hora = (s_last_time.hours * 4)+2;		 
         }
   else {
		 		 avance_hora = (s_last_time.hours * 4)+3;
         }
	
	
  //Put a fixed time
	//s_last_time.hours = 10;
  //s_last_time.minutes = 10;
	//s_last_time.seconds = 27;
	
	
//  for(int i = 0; i < 3; i++) {
//    s_color_channels[i] = rand() % 256;
//  }

  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

static void update_proc(Layer *layer, GContext *ctx) {
	
  // Don't use current time while animating
	Time mode_time = (s_animating) ? s_anim_time : s_last_time;
	GPoint ChronoFin;
	
  // Adjust for minutes through the hour
  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  float hour_angle;
  if(s_animating) {
    // Hours out of 60 for smoothness
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  } else {
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
  }
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  graphics_context_set_antialiased(ctx, ANTIALIASING);

	// Draw Fake chrono
	ChronoFin.x = 34;
	ChronoFin.y = 56;
	graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, s_center_chrono1, ChronoFin);
	graphics_draw_circle(ctx, s_center_chrono1, 3);
	ChronoFin.x = 109;
  graphics_draw_line(ctx, s_center_chrono2, ChronoFin);
	graphics_draw_circle(ctx, s_center_chrono2, 3);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_circle(ctx, s_center_chrono1, 1);
	graphics_draw_circle(ctx, s_center_chrono2, 1);
	//graphics_draw_pixel(ctx, s_center_chrono1);
	//graphics_draw_pixel(ctx, s_center_chrono2);

	
  // Draw hands with positive length only
  if(s_radius > 2 * HAND_MARGIN) {
			graphics_context_set_stroke_color(ctx, GColorBlack);
	    graphics_context_set_stroke_width(ctx, 7);
      //graphics_draw_line(ctx, s_center, Horas[s_last_time.hours]);
      //graphics_draw_line(ctx, s_center, Rabo_Horas[s_last_time.hours]);
      graphics_draw_line(ctx, s_center, Horas[avance_hora]);
      graphics_draw_line(ctx, s_center, Rabo_Horas[avance_hora]);
			graphics_context_set_stroke_color(ctx, GColorLightGray);
    	graphics_context_set_stroke_width(ctx, 3);
      //graphics_draw_line(ctx, s_center, Horas_corta[s_last_time.hours]);
      graphics_draw_line(ctx, s_center, Horas_corta[avance_hora]);
		
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Dibujo las %d Horas desde centro: %d, %d",  s_last_time.hours, s_center.x, s_center.y);
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Hasta: %d, %d",  Horas[s_last_time.hours].x, Horas[s_last_time.hours].y);
		
  } 
  if(s_radius > HAND_MARGIN) {
		  graphics_context_set_stroke_color(ctx, GColorBlack);
	    graphics_context_set_stroke_width(ctx, 7);
      graphics_draw_line(ctx, s_center, Minutos[s_last_time.minutes]);
      graphics_draw_line(ctx, s_center, Rabo_Minutos[s_last_time.minutes]);
		  graphics_context_set_stroke_color(ctx, GColorLightGray);
	    graphics_context_set_stroke_width(ctx, 3);
      graphics_draw_line(ctx, s_center, Minutos_corto[s_last_time.minutes]);
  }
  if(s_radius > HAND_MARGIN) {
    graphics_context_set_stroke_color(ctx, GColorRed);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, s_center, Segundos[s_last_time.seconds]);
    graphics_draw_line(ctx, s_center, Rabo_Segundos[s_last_time.seconds]);
  }
}

static void date_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  strftime(s_day_buffer, sizeof(s_day_buffer), "%a", t);
	text_layer_set_text(s_day_label, s_day_buffer);

  strftime(s_num_buffer, sizeof(s_num_buffer), "%d", t);
  text_layer_set_text(s_num_label, s_num_buffer);
//  text_layer_set_text(s_num_label, "99");
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Refresco la fecha ahora: %d:%d:%d",  t->tm_hour,t->tm_min,t->tm_sec);	

}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  //s_center = grect_center_point(&window_bounds);
	s_center.x = 72;
	s_center.y = 72;
	s_center_chrono1.x = 34;
	s_center_chrono1.y = 72;
	s_center_chrono2.x = 109;
	s_center_chrono2.y = 72;
	
	// Create GBitmap, then set to created BitmapLayer
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FONDO_RELOJ);
	s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
	
	// Create Layer for Texts (Day, number and weather) 
	s_date_layer = layer_create(window_bounds);
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_layer, s_date_layer);

  //Create layer for the weather icon
	s_icon_layer = bitmap_layer_create(GRect(0, 145, 40, 23));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));
	
	// Create layer for hands
	s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);

			
	// Create Label for Day of the week 
  s_day_label = text_layer_create(GRect(60, 90, 40, 18));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorClear);
  text_layer_set_text_color(s_day_label, GColorWhite);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

	// Create Label for Number of Day of the month
  s_num_label = text_layer_create(GRect(63, 112, 18, 18));
  text_layer_set_text(s_num_label, s_num_buffer);
  text_layer_set_background_color(s_num_label, GColorClear);
  text_layer_set_text_color(s_num_label, GColorBlack);
  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(s_num_label, GTextAlignmentCenter);
	layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));
	
	// Create temperature Label
  s_weather_label = text_layer_create(GRect(44, 144, 100, 25));
  text_layer_set_background_color(s_weather_label, GColorClear);
  text_layer_set_text_color(s_weather_label, GColorWhite);
  text_layer_set_text_alignment(s_weather_label, GTextAlignmentLeft);
  text_layer_set_font(s_weather_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(s_weather_label, "Loading Weather...");
	layer_add_child(s_date_layer, text_layer_get_layer(s_weather_label));

	// Create city label
  s_city_label = text_layer_create(GRect(0, 25, 144, 25));
  text_layer_set_background_color(s_city_label, GColorClear);
  text_layer_set_text_color(s_city_label, GColorWhite);
  text_layer_set_text_alignment(s_city_label, GTextAlignmentCenter);
  text_layer_set_font(s_city_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text(s_city_label, "City...");
	layer_add_child(s_date_layer, text_layer_get_layer(s_city_label));
		
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  layer_destroy(s_date_layer);
	text_layer_destroy(s_weather_label);

}

/*********************************** App **************************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

static void radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = anim_percentage(dist_normalized, FINAL_RADIUS);

  layer_mark_dirty(s_canvas_layer);
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = anim_percentage(dist_normalized, s_last_time.minutes);
	s_anim_time.seconds = anim_percentage(dist_normalized, s_last_time.seconds);

  layer_mark_dirty(s_canvas_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  static char city_buffer[32];
  static char icon_buffer[8];
	static int temperature;
	static int temp_fahr;

	
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
			temperature = (int)t->value->int32;
//      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", temperature);
			temp_fahr = (temperature / 9) * 5 + 32;
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", temp_fahr);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    case KEY_CITY:
      snprintf(city_buffer, sizeof(city_buffer), "%s", t->value->cstring);
      break;
    case KEY_ICON:
         snprintf(icon_buffer, sizeof(icon_buffer), "%d", (int)t->value->int32);
         if (s_icon_bitmap) {
           gbitmap_destroy(s_icon_bitmap);
         }
         s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[t->value->int32]);
//#ifdef PBL_SDK_3
         bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
//#endif
         bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);			
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
		
		APP_LOG(APP_LOG_LEVEL_ERROR, "Responde OpenWeather: " );

  }
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_label, weather_layer_buffer);
  text_layer_set_text(s_city_label, city_buffer);

}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, SECOND_UNIT);

	Inicia_Arrays_Pixeles();
	
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  // Prepare animations
   AnimationImplementation radius_impl = {
    .update = radius_update
  };
  animate(ANIMATION_DURATION, ANIMATION_DELAY, &radius_impl, false);

  AnimationImplementation hands_impl = {
    .update = hands_update
  };
  animate(2 * ANIMATION_DURATION, ANIMATION_DELAY, &hands_impl, true);
	
	  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
