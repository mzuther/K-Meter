<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fn="http://www.w3.org/2005/04/xpath-functions" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xlink="http://www.w3.org/1999/xlink" exclude-result-prefixes="xsl fn xs">

	<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-public="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd" media-type="text/xhtml" omit-xml-declaration="no" version="4.01" />


	<xsl:template match="/">
		<html lang="en">
			<head>
				<title><xsl:value-of select="/*/@name" /></title>

				<style type="text/css">
					<xsl:choose>
						<xsl:when test="/*/@orientation = 'vertical'">
							th { text-align: center; vertical-align: top; font-weight: bold; }
							th span { font-weight: normal; }
							td { text-align: center; vertical-align: top; font-weight: bold; }
						</xsl:when>

						<xsl:otherwise>
							th { text-align: left; vertical-align: top; font-weight: bold; }
							th span { font-weight: normal; }
							td { text-align: left; vertical-align: top; font-weight: bold; }
						</xsl:otherwise>
					</xsl:choose>
				</style>
			</head>

			<body>
				<h1>K-Meter Skin &quot;<xsl:value-of select="/*/@name" />&quot;</h1>

				<xsl:call-template name="draw_meters">
					<xsl:with-param name="configuration" select="'stereo'" />
					<xsl:with-param name="meter-mode" select="'itu'" />
					<xsl:with-param name="fallback_1" select="/*/stereo_itu" />
					<xsl:with-param name="fallback_2" select="/*/default" />
				</xsl:call-template>

				<xsl:call-template name="draw_meters">
					<xsl:with-param name="configuration" select="'stereo'" />
					<xsl:with-param name="meter-mode" select="'rms'" />
					<xsl:with-param name="fallback_1" select="/*/stereo_rms" />
					<xsl:with-param name="fallback_2" select="/*/default" />
				</xsl:call-template>

				<xsl:call-template name="draw_meters">
					<xsl:with-param name="configuration" select="'surround'" />
					<xsl:with-param name="meter-mode" select="'itu'" />
					<xsl:with-param name="fallback_1" select="/*/surround_itu" />
					<xsl:with-param name="fallback_2" select="/*/default" />
				</xsl:call-template>

				<xsl:call-template name="draw_meters">
					<xsl:with-param name="configuration" select="'surround'" />
					<xsl:with-param name="meter-mode" select="'rms'" />
					<xsl:with-param name="fallback_1" select="/*/surround_rms" />
					<xsl:with-param name="fallback_2" select="/*/default" />
				</xsl:call-template>
			</body>
		</html>
	</xsl:template>


	<xsl:template name="draw_meters">
		<xsl:param name="configuration" />
		<xsl:param name="meter-mode" />
		<xsl:param name="fallback_1" />
		<xsl:param name="fallback_2" />

		<xsl:variable name="meter_mode_label">
			<xsl:choose>
				<xsl:when test="$meter-mode = 'rms'">RMS</xsl:when>

				<xsl:otherwise>ITU-R</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<h2>
			<xsl:value-of select="concat(upper-case(substring($configuration, 1, 1)), substring($configuration, 2)), ' ('" />
			<xsl:value-of select="$meter_mode_label" />)
		</h2>

		<xsl:choose>
			<xsl:when test="/*/@orientation = 'vertical'">
				<table>
					<tbody>
						<tr>
							<th>
								K-20 (<xsl:value-of select="$meter_mode_label" />)<br />
								<span>no peaks</span>
							</th>

							<th>
								K-14 (<xsl:value-of select="$meter_mode_label" />)<br />
								<span>peaks</span>
							</th>

							<th>
								K-12 (<xsl:value-of select="$meter_mode_label" />)<br />
								<span>no peaks</span>
							</th>

							<th>
								Normal (<xsl:value-of select="$meter_mode_label" />)<br />
								<span>no peaks</span>
							</th>
						</tr>

						<tr>
							<td>
								<xsl:call-template name="draw_single_meter">
									<xsl:with-param name="node" select="root($fallback_2)/*/*[local-name() = string-join(($configuration, $meter-mode, 'k20'), '_')]" />
									<xsl:with-param name="fallback_1" select="$fallback_1" />
									<xsl:with-param name="fallback_2" select="$fallback_2" />
									<xsl:with-param name="configuration" select="$configuration" />
									<xsl:with-param name="meter-mode" select="$meter-mode" />
									<xsl:with-param name="crest-factor" select="'k20'" />
								</xsl:call-template>
							</td>

							<td>
								<xsl:call-template name="draw_single_meter">
									<xsl:with-param name="node" select="root($fallback_2)/*/*[local-name() = string-join(($configuration, $meter-mode, 'k14'), '_')]" />
									<xsl:with-param name="fallback_1" select="$fallback_1" />
									<xsl:with-param name="fallback_2" select="$fallback_2" />
									<xsl:with-param name="configuration" select="$configuration" />
									<xsl:with-param name="meter-mode" select="$meter-mode" />
									<xsl:with-param name="crest-factor" select="'k14'" />
								</xsl:call-template>
							</td>

							<td>
								<xsl:call-template name="draw_single_meter">
									<xsl:with-param name="node" select="root($fallback_2)/*/*[local-name() = string-join(($configuration, $meter-mode, 'k12'), '_')]" />
									<xsl:with-param name="fallback_1" select="$fallback_1" />
									<xsl:with-param name="fallback_2" select="$fallback_2" />
									<xsl:with-param name="configuration" select="$configuration" />
									<xsl:with-param name="meter-mode" select="$meter-mode" />
									<xsl:with-param name="crest-factor" select="'k12'" />
								</xsl:call-template>
							</td>

							<td>
								<xsl:call-template name="draw_single_meter">
									<xsl:with-param name="node" select="root($fallback_2)/*/*[local-name() = string-join(($configuration, $meter-mode, 'normal'), '_')]" />
									<xsl:with-param name="fallback_1" select="$fallback_1" />
									<xsl:with-param name="fallback_2" select="$fallback_2" />
									<xsl:with-param name="configuration" select="$configuration" />
									<xsl:with-param name="meter-mode" select="$meter-mode" />
									<xsl:with-param name="crest-factor" select="'normal'" />
								</xsl:call-template>
							</td>
						</tr>
					</tbody>
				</table>
			</xsl:when>

			<xsl:otherwise>
				<table>
					<tbody>
						<tr>
							<th>
								K-20 (<xsl:value-of select="$meter_mode_label" />)<br />
								<span>no peaks</span>
							</th>

							<td>
								<xsl:call-template name="draw_single_meter">
									<xsl:with-param name="node" select="root($fallback_2)/*/*[local-name() = string-join(($configuration, $meter-mode, 'k20'), '_')]" />
									<xsl:with-param name="fallback_1" select="$fallback_1" />
									<xsl:with-param name="fallback_2" select="$fallback_2" />
									<xsl:with-param name="configuration" select="$configuration" />
									<xsl:with-param name="meter-mode" select="$meter-mode" />
									<xsl:with-param name="crest-factor" select="'k20'" />
								</xsl:call-template>
							</td>
						</tr>

						<tr>
							<th>
								K-14 (<xsl:value-of select="$meter_mode_label" />)<br />
								<span>peaks</span>
							</th>

							<td>
								<xsl:call-template name="draw_single_meter">
									<xsl:with-param name="node" select="root($fallback_2)/*/*[local-name() = string-join(($configuration, $meter-mode, 'k14'), '_')]" />
									<xsl:with-param name="fallback_1" select="$fallback_1" />
									<xsl:with-param name="fallback_2" select="$fallback_2" />
									<xsl:with-param name="configuration" select="$configuration" />
									<xsl:with-param name="meter-mode" select="$meter-mode" />
									<xsl:with-param name="crest-factor" select="'k14'" />
								</xsl:call-template>
							</td>
						</tr>

						<tr>
							<th>
								K-12 (<xsl:value-of select="$meter_mode_label" />)<br />
								<span>no peaks</span>
							</th>

							<td>
								<xsl:call-template name="draw_single_meter">
									<xsl:with-param name="node" select="root($fallback_2)/*/*[local-name() = string-join(($configuration, $meter-mode, 'k12'), '_')]" />
									<xsl:with-param name="fallback_1" select="$fallback_1" />
									<xsl:with-param name="fallback_2" select="$fallback_2" />
									<xsl:with-param name="configuration" select="$configuration" />
									<xsl:with-param name="meter-mode" select="$meter-mode" />
									<xsl:with-param name="crest-factor" select="'k12'" />
								</xsl:call-template>
							</td>

							<tr>
								<th>
									Normal (<xsl:value-of select="$meter_mode_label" />)<br />
									<span>no peaks</span>
								</th>

								<td>
									<xsl:call-template name="draw_single_meter">
										<xsl:with-param name="node" select="root($fallback_2)/*/*[local-name() = string-join(($configuration, $meter-mode, 'normal'), '_')]" />
										<xsl:with-param name="fallback_1" select="$fallback_1" />
										<xsl:with-param name="fallback_2" select="$fallback_2" />
										<xsl:with-param name="configuration" select="$configuration" />
										<xsl:with-param name="meter-mode" select="$meter-mode" />
										<xsl:with-param name="crest-factor" select="'normal'" />
									</xsl:call-template>
								</td>
							</tr>
						</tr>
					</tbody>
				</table>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<xsl:template name="draw_single_meter">
		<xsl:param name="node" />
		<xsl:param name="fallback_1" />
		<xsl:param name="fallback_2" />
		<xsl:param name="configuration" />
		<xsl:param name="meter-mode" />
		<xsl:param name="crest-factor" />

		<svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="{$node/background/@width}" height="{$node/background/@height}">
			<rect x="0" y="0" width="{$node/background/@width}" height="{$node/background/@height}" style="fill:silver;" />

			<image x="0" y="0" width="{$node/background/@width}" height="{$node/background/@height}">
				<xsl:choose>
					<xsl:when test="$crest-factor = 'k14'">
						<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($node)/*/@path, '/', $node/background/@image_peaks)" /></xsl:attribute>
					</xsl:when>

					<xsl:otherwise>
						<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($node)/*/@path, '/', $node/background/@image_no_peaks)" /></xsl:attribute>
					</xsl:otherwise>
				</xsl:choose>
			</image>

			<xsl:for-each select="tokenize('button_k20 | button_k14 | button_k12 | button_normal | button_itu | button_rms | button_hold | button_peaks | button_expand | button_skin | button_mono | button_reset | button_validate | button_about', '\s*\|\s*')">
				<xsl:variable name="button">
					<xsl:value-of select="string(.)" />
				</xsl:variable>

				<xsl:call-template name="component_active">
					<xsl:with-param name="node" select="$node/*[local-name() = $button]" />
					<xsl:with-param name="fallback_1" select="$fallback_1/*[local-name() = $button]" />
					<xsl:with-param name="fallback_2" select="$fallback_2/*[local-name() = $button]" />
					<xsl:with-param name="component_state">
						<xsl:choose>
							<xsl:when test="ends-with($button, $meter-mode)">on</xsl:when>

							<xsl:when test="ends-with($button, $crest-factor)">on</xsl:when>

							<xsl:when test="$crest-factor = 'k20'">
								<xsl:choose>
									<xsl:when test="contains($button, 'hold')">on</xsl:when>

									<xsl:when test="contains($button, 'mono')">on</xsl:when>

									<xsl:otherwise>off</xsl:otherwise>
								</xsl:choose>
							</xsl:when>

							<xsl:when test="$crest-factor = 'k14'">
								<xsl:choose>
									<xsl:when test="contains($button, 'peaks')">on</xsl:when>

									<xsl:when test="contains($button, 'reset')">on</xsl:when>

									<xsl:otherwise>off</xsl:otherwise>
								</xsl:choose>
							</xsl:when>

							<xsl:when test="$crest-factor = 'k12'">
								<xsl:choose>
									<xsl:when test="contains($button, 'expand')">on</xsl:when>

									<xsl:when test="contains($button, 'validate')">on</xsl:when>

									<xsl:otherwise>off</xsl:otherwise>
								</xsl:choose>
							</xsl:when>

							<xsl:otherwise>
								<xsl:choose>
									<xsl:when test="contains($button, 'skin')">on</xsl:when>

									<xsl:when test="contains($button, 'about')">on</xsl:when>

									<xsl:otherwise>off</xsl:otherwise>
								</xsl:choose>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>

			<xsl:for-each select="tokenize('label_over | label_over_left | label_over_right | label_over_center | label_over_lfe | label_over_ls | label_over_rs | label_peak | label_peak_left | label_peak_right | label_peak_center | label_peak_lfe | label_peak_ls | label_peak_rs', '\s*\|\s*')">
				<xsl:variable name="label">
					<xsl:value-of select="string(.)" />
				</xsl:variable>

				<xsl:call-template name="component_active">
					<xsl:with-param name="node" select="$node/*[local-name() = $label]" />
					<xsl:with-param name="fallback_1" select="$fallback_1/*[local-name() = $label]" />
					<xsl:with-param name="fallback_2" select="$fallback_2/*[local-name() = $label]" />

					<xsl:with-param name="component_state">
						<xsl:choose>
							<xsl:when test="ends-with($label, '_right') or ends-with($label, '_lfe') or ends-with($label, '_rs')">on</xsl:when>

							<xsl:otherwise>off</xsl:otherwise>
						</xsl:choose>
					</xsl:with-param>
				</xsl:call-template>
			</xsl:for-each>

			<xsl:for-each select="tokenize('meter_kmeter | meter_kmeter_left | meter_kmeter_right | meter_kmeter_center | meter_kmeter_lfe | meter_kmeter_ls | meter_kmeter_rs | meter_phase_correlation | meter_stereo', '\s*\|\s*')">
				<xsl:variable name="meter">
					<xsl:value-of select="string(.)" />
				</xsl:variable>

				<xsl:call-template name="component_meter">
					<xsl:with-param name="node" select="$node/*[local-name() = $meter]" />
					<xsl:with-param name="fallback_1" select="$fallback_1/*[local-name() = $meter]" />
					<xsl:with-param name="fallback_2" select="$fallback_2/*[local-name() = $meter]" />
				</xsl:call-template>
			</xsl:for-each>

			<xsl:if test="$crest-factor = 'k20'">
				<xsl:for-each select="tokenize('label_debug', '\s*\|\s*')">
					<xsl:variable name="label">
						<xsl:value-of select="string(.)" />
					</xsl:variable>

					<xsl:call-template name="component_passive">
						<xsl:with-param name="node" select="$node/*[local-name() = $label]" />
						<xsl:with-param name="fallback_1" select="$fallback_1/*[local-name() = $label]" />
						<xsl:with-param name="fallback_2" select="$fallback_2/*[local-name() = $label]" />
					</xsl:call-template>
				</xsl:for-each>
			</xsl:if>
		</svg>
	</xsl:template>


	<xsl:template name="component_active">
		<xsl:param name="node" />
		<xsl:param name="fallback_1" />
		<xsl:param name="fallback_2" />
		<xsl:param name="component_state" />

		<xsl:choose>
			<xsl:when test="exists($node)">
				<image x="{$node/@x}" y="{$node/@y}" width="{$node/@width}" height="{$node/@height}">
					<xsl:choose>
						<xsl:when test="$component_state = 'on'">
							<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($node)/*/@path, '/', ($node/@image_on))" /></xsl:attribute>
						</xsl:when>

						<xsl:otherwise>
							<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($node)/*/@path, '/', ($node/@image_off))" /></xsl:attribute>
						</xsl:otherwise>
					</xsl:choose>
				</image>
			</xsl:when>

			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="exists($fallback_1)">
						<image x="{$fallback_1/@x}" y="{$fallback_1/@y}" width="{$fallback_1/@width}" height="{$fallback_1/@height}">
							<xsl:choose>
								<xsl:when test="$component_state = 'on'">
									<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_1)/*/@path, '/', ($fallback_1/@image_on))" /></xsl:attribute>
								</xsl:when>

								<xsl:otherwise>
									<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_1)/*/@path, '/', ($fallback_1/@image_off))" /></xsl:attribute>
								</xsl:otherwise>
							</xsl:choose>
						</image>
					</xsl:when>

					<xsl:otherwise>
						<image x="{$fallback_2/@x}" y="{$fallback_2/@y}" width="{$fallback_2/@width}" height="{$fallback_2/@height}">
							<xsl:choose>
								<xsl:when test="$component_state = 'on'">
									<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_2)/*/@path, '/', ($fallback_2/@image_on))" /></xsl:attribute>
								</xsl:when>

								<xsl:otherwise>
									<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_2)/*/@path, '/', ($fallback_2/@image_off))" /></xsl:attribute>
								</xsl:otherwise>
							</xsl:choose>
						</image>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<xsl:template name="component_passive">
		<xsl:param name="node" />
		<xsl:param name="fallback_1" />
		<xsl:param name="fallback_2" />

		<xsl:choose>
			<xsl:when test="exists($node)">
				<image x="{$node/@x}" y="{$node/@y}" width="{$node/@width}" height="{$node/@height}">
					<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($node)/*/@path, '/', ($node/@image))" /></xsl:attribute>
				</image>
			</xsl:when>

			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="exists($fallback_1)">
						<image x="{$fallback_1/@x}" y="{$fallback_1/@y}" width="{$fallback_1/@width}" height="{$fallback_1/@height}">
							<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_1)/*/@path, '/', ($fallback_1/@image))" /></xsl:attribute>
						</image>
					</xsl:when>

					<xsl:otherwise>
						<image x="{$fallback_2/@x}" y="{$fallback_2/@y}" width="{$fallback_2/@width}" height="{$fallback_2/@height}">
							<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_2)/*/@path, '/', ($fallback_2/@image))" /></xsl:attribute>
						</image>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>


	<xsl:template name="component_meter">
		<xsl:param name="node" />
		<xsl:param name="fallback_1" />
		<xsl:param name="fallback_2" />

		<xsl:choose>
			<xsl:when test="exists($node)">
				<image x="{$node/@x}" y="{$node/@y}" width="{$node/@width}" height="{$node/@height}">
					<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($node)/*/@path, '/', ($node/@image))" /></xsl:attribute>
				</image>

				<xsl:if test="exists($node/needle)">
					<image x="{$node/@x + ($node/@width * 0.5) - ($node/needle/@width * 0.5)}" y="{$node/@y + $node/@spacing_top}" width="{$node/needle/@width}" height="{$node/needle/@height}">
						<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($node)/*/@path, '/', ($node/needle/@image))" /></xsl:attribute>
					</image>
				</xsl:if>
			</xsl:when>

			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="exists($fallback_1)">
						<image x="{$fallback_1/@x}" y="{$fallback_1/@y}" width="{$fallback_1/@width}" height="{$fallback_1/@height}">
							<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_1)/*/@path, '/', ($fallback_1/@image))" /></xsl:attribute>
						</image>

						<xsl:if test="exists($fallback_1/needle)">
							<image x="{$fallback_1/@x + ($fallback_1/@width * 0.5) - ($fallback_1/needle/@width * 0.5)}" y="{$fallback_1/@y + $fallback_1/@spacing_top}" width="{$fallback_1/needle/@width}" height="{$fallback_1/needle/@height}">
								<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_1)/*/@path, '/', ($fallback_1/needle/@image))" /></xsl:attribute>
							</image>
						</xsl:if>
					</xsl:when>

					<xsl:otherwise>
						<image x="{$fallback_2/@x}" y="{$fallback_2/@y}" width="{$fallback_2/@width}" height="{$fallback_2/@height}">
							<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_2)/*/@path, '/', ($fallback_2/@image))" /></xsl:attribute>
						</image>

						<xsl:if test="exists($fallback_2/needle)">
							<image x="{$fallback_2/@x + ($fallback_2/@width * 0.5) - ($fallback_2/needle/@width * 0.5)}" y="{$fallback_2/@y + $fallback_2/@spacing_top}" width="{$fallback_2/needle/@width}" height="{$fallback_2/needle/@height}">
								<xsl:attribute name="xlink:href"><xsl:value-of select="concat(root($fallback_2)/*/@path, '/', ($fallback_2/needle/@image))" /></xsl:attribute>
							</image>
						</xsl:if>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

</xsl:stylesheet>
