<?php
/*
 * Gallery - a web based photo album viewer and editor
 * Copyright (C) 2000-2008 Bharat Mediratta
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * Settings for Remote
 * @package Remote
 * @subpackage UserInterface
 * @author Alan Harder <alan.harder@sun.com>
 * @version $Revision: 17580 $
 */
class RemoteSiteAdminController extends GalleryController {
	/**
	 * @see GalleryController::handleRequest
	 */
	public function handleRequest($form) {
		$ret = GalleryCoreApi::assertUserIsSiteAdministrator();

		if ($ret) {
			return array($ret, null);
		}

		$status = $error = array();

		if (isset($form['action']['save'])) {
			foreach (array('showItemAdd') as $key) {
				$ret = GalleryCoreApi::setPluginParameter(
					'module',
					'remote',
					$key,
					(isset($form[$key]) && $form[$key]) ? 1 : 0
				);

				if ($ret) {
					return array($ret, null);
				}
			}
			$status['saved'] = 1;
		}

		$results = array(
			'redirect' => array(
				'view'    => 'core.SiteAdmin',
				'subView' => 'remote.RemoteSiteAdmin',
			),
			'status'   => $status,
			'error'    => $error,
		);

		return array(null, $results);
	}
}

/**
 * Settings for Remote
 */
class RemoteSiteAdminView extends GalleryView {
	/**
	 * @see GalleryView::loadTemplate
	 */
	public function loadTemplate(&$template = null, &$form = null) {
		$ret = GalleryCoreApi::assertUserIsSiteAdministrator();

		if ($ret) {
			return array($ret, null);
		}

		if (!isset($form['showItemAdd'])) {
			$form['showItemAdd'] = false;
		}

		if ($form['formName'] != 'RemoteSiteAdmin') {
			$form['formName']   = 'RemoteSiteAdmin';
			list($ret, $params) = GalleryCoreApi::fetchAllPluginParameters('module', 'remote');

			if ($ret) {
				return array($ret, null);
			}

			foreach ($params as $key => $value) {
				$form[$key] = $value;
			}
		}

		$template->setVariable('controller', 'remote.RemoteSiteAdmin');

		return array(
			null,
			array(
				'body' => 'modules/remote/templates/RemoteSiteAdmin.tpl',
			),
		);
	}
}
