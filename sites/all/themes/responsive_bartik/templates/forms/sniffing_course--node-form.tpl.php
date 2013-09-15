<?php
   $form['comment_settings']['#attributes']['type']='hidden';
	print drupal_render($form['title']);
	print drupal_render($form['body']);
	print drupal_render($form['menu']);
	print drupal_render($form['field_price_per_hour']);
	print drupal_render($form['field_instrument']);
	print drupal_render($form['field_address']);
	print drupal_render($form['field_youtube']);
	print drupal_render($form['field_image_of_course']);
	print drupal_render($form['field_course_on_map']);
	print drupal_render($form['field_venue_rating']);
	print drupal_render($form['notifications']);
	print drupal_render($form['actions']);
	print drupal_render($form['options']);
	// Prints the rest of the form.
	print drupal_render_children($form);
?>