<!--  style.xslt  -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" indent="yes"/>

  <xsl:template match="/">
    <xsl:apply-templates select="AdaptadorRed"/>
  </xsl:template>

  <xsl:template match="AdaptadorRed">
    <html>
      <head>
        <style>
          table { border-collapse: collapse; width: 100%; } th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; } th { background-color: #f2f2f2; }
        </style>
      </head>
      <body>
        <h2>Información del Adaptador de Red</h2>
        <table>
          <tr>
            <th>Propiedad</th>
            <th>Valor</th>
          </tr>
          <tr>
            <td>IP</td>
            <td>
              <xsl:value-of select="IP"/>
            </td>
          </tr>
          <tr>
            <td>Máscara</td>
            <td>
              <xsl:value-of select="Mascara"/>
            </td>
          </tr>
          <tr>
            <td>Puerta de Enlace</td>
            <td>
              <xsl:value-of select="PuertaDeEnlace"/>
            </td>
          </tr>
          <tr>
            <td>Servidor DNS</td>
            <td>
              <xsl:value-of select="ServidorDNS"/>
            </td>
          </tr>
          <tr>
            <td>Velocidad de Respuesta DNS</td>
            <td>
              <xsl:value-of select="VelocidadRespuestaDNS"/>
            </td>
          </tr>
          <tr>
            <td>Número de Saltos</td>
            <td>
              <xsl:value-of select="NumeroSaltos"/>
            </td>
          </tr>
          <tr>
            <td>IP de Saltos</td>
            <td>
              <xsl:apply-templates select="IpSaltos/Salto"/>
            </td>
          </tr>
        </table>
      </body>
    </html>
  </xsl:template>
  <xsl:template match="Salto">
    <xsl:value-of select="."/>
    <br/>
  </xsl:template>
</xsl:stylesheet>
