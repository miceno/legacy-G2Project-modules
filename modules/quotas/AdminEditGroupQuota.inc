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
 * This controller will handle changes made to a group's quotas
 * @package Quotas
 * @subpackage UserInterface
 * @author Robert Balousek <volksport@users.sf.net>
 * @version $Revision: 17580 $
 */
class AdminEditGroupQuotaController extends GalleryController {
	/**
	 * @see GalleryController::handleRequest
	 */
	public function handleRequest($form) {
		$ret = GalleryCoreApi::assertUserIsSiteAdministrator();

		if ($ret) {
			return array($ret, null);
		}

		$results = $status = $error = array();

		$groupId           = GalleryUtilities::getRequestVariables('groupId');
		list($ret, $group) = GalleryCoreApi::loadEntitiesById($groupId, 'GalleryGroup');

		if ($ret) {
			return array($ret, null);
		}

		if (isset($form['action']['cancel'])) {
			// Redirect back to the AdminGroups view
			$redirect['view']    = 'core.SiteAdmin';
			$redirect['subView'] = 'quotas.AdminQuotas';
		} elseif (isset($form['action']['undo'])) {
			// Redirect back to the same view to reset the form
			$redirect['view']    = 'core.SiteAdmin';
			$redirect['subView'] = 'quotas.AdminEditGroupQuota';
			$redirect['groupId'] = $groupId;
		} elseif (isset($form['action']['save'])) {
			if (array_key_exists('quotaSize', $form) && array_key_exists('quotaUnit', $form)) {
				if (!empty($form['quotaSize']) && is_numeric($form['quotaSize'])
					&& $form['quotaSize'] >= 0 && !empty($form['quotaSize'])
				) {
					GalleryCoreApi::requireOnce(
						'modules/quotas/classes/GalleryQuotasHelper.class'
					);
					$form['quotaSize'] = round((float)$form['quotaSize'], 2);

					if ($form['quotaUnit'] == 'KB') {
						$quotaSize = round($form['quotaSize']);
					} elseif ($form['quotaUnit'] == 'MB') {
						$quotaSize = round($form['quotaSize'] * 1024.00);
					} elseif ($form['quotaUnit'] == 'GB') {
						$quotaSize = round($form['quotaSize'] * 1024.00 * 1024.00);
					} else {
						return array(GalleryCoreApi::error(ERROR_MISSING_VALUE), null);
					}

					if ($quotaSize <= 2147483647) {
						if (!isset($group)) {
							return array(GalleryCoreApi::error(ERROR_MISSING_OBJECT), null);
						}

						list($ret, $quotaExists, $groupQuota) = GalleryQuotasHelper::fetchGroupQuota($group->getId());

						if ($ret) {
							return array($ret, null);
						}

						if ($quotaExists) {
							$match = array(
								'userOrGroupId' => $groupQuota['id'],
								'quotaSize'     => $groupQuota['quota'],
							);
							$ret   = GalleryCoreApi::updateMapEntry(
								'QuotasMap',
								$match,
								array(
									'userOrGroupId' => $group->getId(),
									'quotaSize'     => $quotaSize,
								)
							);

							if ($ret) {
								return array($ret, null);
							}

							// Request a redirect to the confirmation screen
							$redirect['view']        = 'core.SiteAdmin';
							$redirect['subView']     = 'quotas.AdminQuotas';
							$status['modifiedGroup'] = $group->getGroupName();
						}
					} else {
						$error[] = 'form[error][quotaSize][tooLarge]';
						$error[] = 'form[error][quotaSize][' . $quotaSize . ']';
					}
				} else {
					$error[] = 'form[error][quotaSize][notNumeric]';
				}
			} else {
				// set our error status and fall back to the view
				if (empty($form['quotaSize'])) {
					$error[] = 'form[error][quotaSize][missing]';
				}

				if (empty($form['quotaUnit'])) {
					$error[] = 'form[error][quotaUnit][missing]';
				}
			}
		}

		if (!empty($redirect)) {
			$results['redirect'] = $redirect;
		} else {
			$results['delegate']['view']    = 'core.SiteAdmin';
			$results['delegate']['subView'] = 'quotas.AdminEditGroupQuota';
		}
		$results['status'] = $status;
		$results['error']  = $error;

		return array(null, $results);
	}
}

/**
 * This view will show options available to group quotas
 */
class AdminEditGroupQuotaView extends GalleryView {
	/**
	 * @see GalleryView::loadTemplate
	 */
	public function loadTemplate(&$template = null, &$form = null) {
		$ret = GalleryCoreApi::assertUserIsSiteAdministrator();

		if ($ret) {
			return array($ret, null);
		}

		$groupId           = GalleryUtilities::getRequestVariables('groupId');
		list($ret, $group) = GalleryCoreApi::loadEntitiesById($groupId, 'GalleryGroup');

		if ($ret) {
			return array($ret, null);
		}

		GalleryCoreApi::requireOnce('modules/quotas/classes/GalleryQuotasHelper.class');

		list($ret, $quotaExists, $groupQuota) = GalleryQuotasHelper::fetchGroupQuota($group->getId());

		if ($ret) {
			return array($ret, null);
		}

		if ($form['formName'] != 'AdminEditGroupQuota') {
			// First time around initialize our form
			$form['formName'] = 'AdminEditGroupQuota';
		}

		$form['groupName']                                 = $group->getGroupName();
		list($ret, $form['quotaSize'], $form['quotaUnit']) = GalleryQuotasHelper::humanReadableFromKilobytes($groupQuota['quota']);

		if ($ret) {
			return array($ret, null);
		}

		$AdminEditGroupQuota            = array();
		$AdminEditGroupQuota['groupId'] = $group->getId();
		list($ret, $module)             = GalleryCoreApi::loadPlugin('module', 'quotas');

		if ($ret) {
			return array($ret, null);
		}
		$AdminEditGroupQuota['quotaUnitChoices'] = array(
			'KB' => $module->translate('KB'),
			'MB' => $module->translate('MB'),
			'GB' => $module->translate('GB'),
		);

		$template->setVariable('AdminEditGroupQuota', $AdminEditGroupQuota);
		$template->setVariable('controller', 'quotas.AdminEditGroupQuota');

		return array(
			null,
			array(
				'body' => 'modules/quotas/templates/AdminEditGroupQuota.tpl',
			),
		);
	}
}
