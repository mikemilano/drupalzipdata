
Zipdata is a module for developers, not end users.  The only UI provided is an 
admin interface to upload CSV files provided by zip-codes.com. This is a 
subscription service which provides the latest zipcode data. I have no 
affiliation with them.

The primary purpose of this module is to provide a way to import zip data into
the Drupal DB, and to gather range info based on a specific zip code. Range 
query results are cached until a new CSV has been uploaded.

A few other simple functions are provided to pull in additional data, however
you can just query the table directly for what you need.

Examples:
---------
// Get zip codes within 25 miles of 90210.
$zipcodes = zipdata_query_radius($zipcode, 25);

// You may use something like this to get all the node ids with a ip code
// within 25 miles of 90210.
-------------------------------------------------------------------------------
$nids = array();
$geodata = zipdata_query_radius(90210, 25, 'm');
if (count($geodata) > 0) {
  $zipcodes = array();
  foreach ($geodata as $g) {
    $zipcodes[] = $g->zipcode;
  }
  $query = node_select('node', 'n')->fields('n', array('nid'));
  $query->join('field_data_field_ca_zipcode', 'zip', 'zip.entity_id = n.nid ');
  $query->condition('zip.field_ca_zipcode_value', $zipcodes, 'IN');
  $nids = $query->execute()->fetchCol();
}
$nodes = node_load_multiple($nids);
-------------------------------------------------------------------------------

// Get zip codes within 25 km of 90210
$zipcodes = zipdata_query_radius($zipcode, 25, 'km');

Note: The results of zipdata_query_radius() are cached to avoid the heavy SQL
calculations from processing over and over. You can use the following function
to bypass the cache results.

// Get zip codes within 25 miles of 90210 ignoring cache
$zipcodes = zipdata_query_radius($zipcode, 25, 'm', TRUE);

// Get all cities for 90210
$cities = zipdata_get_cities(90210);

// Get all counties for 90210
$counties = zipdata_get_counties($zipcode);

// Get all states for 90210
$states = zipdata_get_states($zipcode);

Author
------
Mike Milano