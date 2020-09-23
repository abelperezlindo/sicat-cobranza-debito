<?php

function procesar_archivo_form($form, &$form_state)
{
    $lastProcessed = variable_get('cobranza_last_processed', 0);

    if ($lastProcessed) {

        $file         = file_load($lastProcessed);
        $uploader     = user_load($file->uid);
        $dateUpload   = date('d/m/Y H:i', $file->timestamp);
        $message      = 'Archivo <b>' . $file->filename . '</b> subido por <b>' . $uploader->name . '</b> en la fecha <b>' . $dateUpload . '</b>';

        $form['info_box'] = [
            '#type'  => 'fieldset',
            '#title' => t('Ultimo procesamiento')
        ];
        $form['info_box']['message'] = [
            '#markup' => $message,
        ];
    }
    $form['file_box'] = [
        '#type'  => 'fieldset',
        '#title' => t('Subir un archivo')
    ];
    $form['file_box']['upload'] = [
        '#type'               => 'managed_file',
        '#title'              => 'Archivo de texto',
        '#name'               => 'archivo_txt',
        '#progress_indicator' => 'bar',
        '#description'        => t('Solo se permite la subida de un archivo de texto (txt) con un tamaño maximo de 2 Mb.'),
        '#upload_location'    => 'public://cobranza/',
        '#upload_validators'  => array(
            'file_validate_extensions'    => array('txt'),
            'file_validate_size'          => [2097152], // 2MB in Bytes
        ),
    ];
    $form['process'] = [
        '#type'     => 'submit',
        '#value'    => 'PROCESAR',
        '#title'    => 'PROCESAR',
        '#validate' => ['procesar_archivo_validate'],
        '#submit'   => ['procesar_archivo_submit']
    ];
    return $form;
}

function procesar_archivo_validate($form, &$form_state)
{
    if (empty($form_state['values']['upload'])) {
        form_set_error('upload', t('DEBE CARGAR UN ARCHIVO.'));
    }
}
function procesar_archivo_submit($form, &$form_state)
{
    $fid = $form_state['values']['upload'];
    if (empty($fid)) {

        form_set_error('upload', t('DEBE CARGAR UN ARCHIVO.'));
        return;
    }

    $file         = file_load($fid); // load temporary file
    $file->status = FILE_STATUS_PERMANENT; //make permanent

    file_save($file);

    $success = processTextFile($file->uri); //process file

    if (!$success) {

        file_delete($file);
        form_set_error('upload', t('ERROR AL PROCESAR'));
        drupal_set_message(t('ERROR AL PROCESAR'), 'error');
        return;
    }

    variable_set('cobranza_last_processed', $file->fid);
    unset($form_state['values']['upload']);
}

function processTextFile($uri)
{
    $handle = fopen($uri, 'r');
    if (!$handle) return false; // error opening the file.

    while (($line = fgets($handle)) !== false) {
        // process the line read.
        drupal_set_message(strlen($line) . ' --- ' . $line);
    }

    fclose($handle);
    return true;
}
