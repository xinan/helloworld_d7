<?php 

/**
 * @file
 * Implements the Mass Import User Record form
 */

namespace CVWO\HelloWorld {
  require_once drupal_get_path('module', MODULE_NAME) . '/helloworld_d7_api.inc';

  /**
   * Define the import form
   */
  function helloworld_import_form() {
    $example = '<ul><li>One record per line.</li>' .
               '<li>Each line consists of a user’s name followed by their email address. A comma separates the name from the email.</li>' .
               '<li>Note that there should be NO space between the comma and the email address.</li></ul>' .
               '<p>For example:</p>' .
               '<pre>John Doe,johndoe@example.com<br>' .
               'Jane Doe,janedoe@example.com<br>' .
               'Jane Roe,janeroe@example.com</pre>';
               
    $form['#attributes']['enctype'] = 'multipart/form-data';
    $form['guide'] = array(
      '#type' => 'fieldset',
      '#title' => t('How should my data be formatted?'),
      '#description' => t('Data should be in the following format:'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['guide']['example'] = array(
      '#type' => 'markup',
      '#markup' => $example,
    );
    $form['import_method'] = array(
      '#type' => 'radios',
      '#options' => drupal_map_assoc(array(t('Upload data file'), t('Enter data directly'))),
      '#title' => t('Choose your import method'),
      '#required' => TRUE,
    );
    $form['file_upload'] = array(
      '#type' => 'file',
      '#title' => t('Upload user data file'),
      '#description' => t('File should be of .txt format.'),
      '#states' => array(
        'visible' => array(
          ':input[name="import_method"]' => array('value' => t('Upload data file')),
          ),
        'required' => array(
          ':input[name="import_method"]' => array('value' => t('Upload data file')),
          ),
        ),
    );
    $form['data'] = array(
      '#type' => 'textarea',
      '#title' => t('Enter data below'),
      '#rows' => 12,
      '#states' => array(
        'visible' => array(
          ':input[name="import_method"]' => array('value' => t('Enter data directly')),
          ),
        'required' => array(
          ':input[name="import_method"]' => array('value' => t('Enter data directly')),
          ),
        ),
    );
    $form['comment'] = array(
      '#type' => 'textarea',
      '#title' => t('Comment'),
      '#rows' => 3,
      '#required' => FALSE,
    );
    $form['import'] = array(
      '#type' => 'submit',
      '#value' => 'Import',
    );
    return $form;
  }

  /**
   * Validate the import form
   */
  function helloworld_import_form_validate($form, &$form_state) {

    if ($form_state['values']['import_method'] == t('Upload data file')) {
      $method = 'file_upload';
      $file = file_save_upload('file_upload', array(
        'file_validate_extensions' => array('txt'),
        ));
      if ($file) {
        $data = file_get_contents($file->uri);
        file_delete($file);
      } else {
        form_set_error('file_upload', t('No file uploaded.'));
      }
    } elseif ($form_state['values']['import_method'] == t('Enter data directly')) {
      $method = 'data';
      $data = $form_state['values']['data'];
    } else {
      $method = 'import_method';
    }

    if (empty($data)) {
      form_set_error($method, t('No data received.'));
    } else {
      // can match different types of line feeds, \n, \r and \r\n
      $entries = preg_split('/\n|\r/', $data, -1, PREG_SPLIT_NO_EMPTY);
      $form_state['helloworld']['entries'] = [];
      foreach ($entries as $entry) {
        $parts = explode(',', $entry);
        if (count($parts) !== 2) {
          form_set_error($method, t('The format of data is incorrect.'));
          break;
        } elseif (!valid_email_address($parts[1])) {
          form_set_error($method, t('Data contains invalid email address(es).'));
          break;
        } else {
          $entry = array(
            'name' => $parts[0],
            'email' => $parts[1],
            'comment' => $form_state['values']['comment'],
            );
          // pass entries to submit function to reduce duplicate work
          $form_state['helloworld']['entries'][] = $entry;
        }
      }
    } 
  }

  /**
   * Submit the import form
   */
  function helloworld_import_form_submit($form, &$form_state) {
    $amount = import_users($form_state['helloworld']['entries']);
    if ($amount) {
      drupal_set_message(t('%amount users successfully imported.', array('%amount' => $amount)));
      $form_state['rebuild'] = TRUE;
    } else {
      drupal_set_message(t('An error occured when importing users.'), 'error');
      $form_state['redirect'] = MODULE_NAME;
    }
  }

}