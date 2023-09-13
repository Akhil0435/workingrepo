/*
 *
 *  bluez-tools - a set of tools to manage bluetooth devices for linux
 *
 *  Copyright (C) 2010  Alexander Orlenko <zxteam@gmail.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gio/gio.h>
#include <glib.h>
#include <string.h>

#include "../dbus-common.h"
#include "../properties.h"

#include "proximity_monitor.h"

#define PROXIMITY_MONITOR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PROXIMITY_MONITOR_TYPE, ProximityMonitorPrivate))

struct _ProximityMonitorPrivate {
	GDBusProxy *proxy;
	Properties *properties;
	gchar *object_path;
};

G_DEFINE_TYPE_WITH_PRIVATE(ProximityMonitor, proximity_monitor, G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_DBUS_OBJECT_PATH /* readwrite, construct only */
};

static void _proximity_monitor_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _proximity_monitor_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _proximity_monitor_create_gdbus_proxy(ProximityMonitor *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void proximity_monitor_dispose(GObject *gobject)
{
	ProximityMonitor *self = PROXIMITY_MONITOR(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	/* Properties free */
	g_clear_object(&self->priv->properties);
	/* Object path free */
	g_free(self->priv->object_path);
	/* Chain up to the parent class */
	G_OBJECT_CLASS(proximity_monitor_parent_class)->dispose(gobject);
}

static void proximity_monitor_finalize (GObject *gobject)
{
	ProximityMonitor *self = PROXIMITY_MONITOR(gobject);
	G_OBJECT_CLASS(proximity_monitor_parent_class)->finalize(gobject);
}

static void proximity_monitor_class_init(ProximityMonitorClass *klass)
{
	g_print("proximity_monitor.c started2\n");
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = proximity_monitor_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;
	g_print("proximity_monitor.c signal sent through get\n");
	gobject_class->get_property = _proximity_monitor_get_property;
	g_print("proximity_monitor.c signal sent through \n");
	gobject_class->set_property = _proximity_monitor_set_property;
	
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "ProximityMonitor D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	if (pspec)
		g_param_spec_unref(pspec);
}

static void proximity_monitor_init(ProximityMonitor *self)
{
	g_print("proximity_monitor.c started\n");
	g_print("proximity_monitor.c get signal sent 2\n");
	self->priv = proximity_monitor_get_instance_private (self);
	self->priv->proxy = NULL;
	self->priv->properties = NULL;
	self->priv->object_path = NULL;
	g_assert(system_conn != NULL);
}

static void _proximity_monitor_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ProximityMonitor *self = PROXIMITY_MONITOR(object);

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		g_print("proximity_monitor.c signal sent g_value_set_string\n")
		g_value_set_string(value, proximity_monitor_get_dbus_object_path(self));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _proximity_monitor_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	ProximityMonitor *self = PROXIMITY_MONITOR(object);
	GError *error = NULL;

	switch (property_id) {
	case PROP_DBUS_OBJECT_PATH:
		self->priv->object_path = g_value_dup_string(value);
		_proximity_monitor_create_gdbus_proxy(self, PROXIMITY_MONITOR_DBUS_SERVICE, self->priv->object_path, &error);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}

	if (error != NULL)
		g_critical("%s", error->message);

	g_assert(error == NULL);
}

/* Constructor */
ProximityMonitor *proximity_monitor_new(const gchar *dbus_object_path)
{
	return g_object_new(PROXIMITY_MONITOR_TYPE, "DBusObjectPath", dbus_object_path, NULL);
}

/* Private DBus proxy creation */
static void _proximity_monitor_create_gdbus_proxy(ProximityMonitor *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync(system_conn, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, PROXIMITY_MONITOR_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", "system", "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
}

/* Methods */

/* Get DBus object path */
const gchar *proximity_monitor_get_dbus_object_path(ProximityMonitor *self)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	g_assert(self->priv->proxy != NULL);
	g_print("proximity_monitor.c get signal sent 3\n");
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}



/* Properties access methods */
GVariant *proximity_monitor_get_properties(ProximityMonitor *self, GError **error)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	g_assert(self->priv->properties != NULL);
	g_print("proximity_monitor.c get all signal sent \n");
	return properties_get_all(self->priv->properties, PROXIMITY_MONITOR_DBUS_INTERFACE, error);
}

void proximity_monitor_set_property(ProximityMonitor *self, const gchar *name, const GVariant *value, GError **error)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	g_assert(self->priv->properties != NULL);
	g_print("proximity_monitor.c set signal sent through properties_set\n")
	properties_set(self->priv->properties, PROXIMITY_MONITOR_DBUS_INTERFACE, name, value, error);
}

const gchar *proximity_monitor_get_immediate_alert_level(ProximityMonitor *self, GError **error)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	g_assert(self->priv->properties != NULL);
	g_print("proximity_monitor.c get signal sent 4\n");
	GVariant *prop = properties_get(self->priv->properties, PROXIMITY_MONITOR_DBUS_INTERFACE, "ImmediateAlertLevel", error);
	if(prop == NULL)
		return NULL;
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

void proximity_monitor_set_immediate_alert_level(ProximityMonitor *self, const gchar *value, GError **error)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	g_assert(self->priv->properties != NULL);
	g_print("proximity_monitor.c signal sent through propeties_set 2\n")
	properties_set(self->priv->properties, PROXIMITY_MONITOR_DBUS_INTERFACE, "ImmediateAlertLevel", g_variant_new_string(value), error);
}

const gchar *proximity_monitor_get_link_loss_alert_level(ProximityMonitor *self, GError **error)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	g_assert(self->priv->properties != NULL);
	g_print("proximity_monitor.c get signal sent 5\n");
	GVariant *prop = properties_get(self->priv->properties, PROXIMITY_MONITOR_DBUS_INTERFACE, "LinkLossAlertLevel", error);
	if(prop == NULL)
		return NULL;
	g_print("proximity_monitor.c get signal sent 6\n");
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

void proximity_monitor_set_link_loss_alert_level(ProximityMonitor *self, const gchar *value, GError **error)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	g_assert(self->priv->properties != NULL);
	g_print("proximity_monitor.c signal sent through prperties_set 3\n")
	properties_set(self->priv->properties, PROXIMITY_MONITOR_DBUS_INTERFACE, "LinkLossAlertLevel", g_variant_new_string(value), error);
}

const gchar *proximity_monitor_get_signal_level(ProximityMonitor *self, GError **error)
{
	g_assert(PROXIMITY_MONITOR_IS(self));
	g_assert(self->priv->properties != NULL);
	g_print("proximity_monitor.c get signal sent 7\n");
	GVariant *prop = properties_get(self->priv->properties, PROXIMITY_MONITOR_DBUS_INTERFACE, "SignalLevel", error);
	if(prop == NULL)
		return NULL;
	g_print("proximity_monitor.c get signal sent 8\n");
	const gchar *ret = g_variant_get_string(prop, NULL);
	g_variant_unref(prop);
	return ret;
}

