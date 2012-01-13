<?php
/**
 * @file
 * Admin callbacks for the zipdata module.
 */

/**
 * Zip data import form.
 */
function zipdata_import($form, &$form_state) {
  $form['zipdata_file'] = array(
    '#type' => 'file',
    '#title' => t('Zipcode Data File'),
    '#description' => t('CSV download from http://www.zip-codes.com/'),
  );
  $form['copy'] = array(
    '#markup' => t('Alternatively to uploading, you can specify where the CSV file currently exists relative to your site files directory.')
  );
  $form['zipdata_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to Zipcode Data File'),
    '#description' => t('The path relative to your site files directory where the Zipcode data CSV file is located.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import Zipcode Data')
  );
  return $form;
}

/**
 * Validate handler for import
 */
function zipdata_import_validate($form, &$form_state) {

  $file = NULL;
  if ($form_state['values']['zipdata_path']) {

  }
  else {
    $validators = array('file_validate_extensions' => array('csv'));
    $file = file_save_upload('zipdata_file', $validators);
    if (!$file) {
      form_set_error('fid', t('Unable to access file or file is missing.'));
      return;
    }

    $form_state['zipdata']['zipdata_file'] = $file;
  }
  // store file
  $form_state['zipdata']['zipdata_file'] = $file;
  // get actual file path from uri
  $filepath = drupal_realpath($file->uri);

  // csv header keys
  $keys = array(
    'ZipCode' => -1,
    'State' => -1,
    'AreaCode' => -1,
    'Latitude' => -1,
    'Longitude' => -1,
    'CityAliasName' => -1,
    'County' => -1
  );

  // open CSV file
  if (($handle = fopen($filepath, "r")) !== FALSE) {
    // read headers into first line
    $data = fgetcsv($handle, 1000, ",");
    // set $keys values to actual data position of each column
    foreach ($data as $key => $value) {
      if (key_exists($value, $keys)) {
        $keys[$value] = $key;
      }
    }
    fclose($handle);

    // make sure all headers are accounted for
    foreach ($keys as $key => $value) {
      if ($value < 0) {
        form_set_error('zipdata_file', 'Invalid CSV Format. '. $key .' column not found.');
        return;
      }
    }
    // store keys
    $form_state['zipdata']['keys'] = $keys;
  }
  else {
    form_set_error('zipdata_file', 'Can not open file.');
    return;
  }
}

/**
 * Submit handler for import
 */
function zipdata_import_submit(&$form, &$form_state) {
  // store file
  $file = $form_state['zipdata']['zipdata_file'];
  // get actual file path from uri
  $filepath = drupal_realpath($file->uri);
  // get keys
  $keys = $form_state['zipdata']['keys'];
  // clear out the zipdata table
  db_query("TRUNCATE TABLE {zipdata}");
  // open csv file
  $i = 0;
  if (($handle = fopen($filepath, "r")) !== FALSE) {
    // advance past header
    $data = fgetcsv($handle, 1000, ",");
    // loop through data for db inserts
    while (($data = fgetcsv($handle, 1000, ",")) !== FALSE) {
      db_query("INSERT INTO {zipdata} (zipcode,city,county,state,latitude,longitude) VALUES (:zipcode, :city, :county, :state, :latitude, :longitude)",
                  array(':zipcode' => $data[$keys['ZipCode']],
                        ':city' => $data[$keys['CityAliasName']],
                        ':county' => $data[$keys['County']],
                        ':state' => $data[$keys['State']],
                        ':latitude' => $data[$keys['Latitude']],
                        ':longitude' => $data[$keys['Longitude']],
                       ));
      $i++;
    }
    fclose($handle);
    // set insert message
    drupal_set_message($i .' zipcode records entered.');
  }
  // cleanup file
  file_delete($file);
  // clear zip radius cache cache
  cache_clear_all(NULL, 'cache_zipdata_radius');
  // set cache message
  drupal_set_message('Zipcode radius cache cleared.');
}

/**
 * Zip data rebuild cache form.
 */
function zipdata_clear_cache(&$form_state) {
  $count = db_query("SELECT COUNT(*) FROM {cache_zipdata_radius}")->fetchField();
  $form['copy'] = array(
    '#markup' => '<p>'. t(':count cached records found.', array(':count' => $count)) .'</p>'
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Clear Zipcode Data Cache'),
  );
  return $form;
}

/**
 * Zip data rebuild cache form.
 */
function zipdata_clear_cache_submit(&$form, &$form_state) {
  // clear zip radius cache cache
  db_query("TRUNCATE TABLE {cache_zipdata_radius}");
  // set cache message
  drupal_set_message('Zipcode radius cache cleared.');
}