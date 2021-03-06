/*
 * This file is part of cf4ocl (C Framework for OpenCL).
 *
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * cf4ocl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with cf4ocl. If not, see
 * <http://www.gnu.org/licenses/>.
 * */

/**
 * @file
 *
 * Classes and functions for filtering and selecting OpenCL contexts
 * and associated devices.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_DEVICE_SELECTOR_H_
#define _CCL_DEVICE_SELECTOR_H_

#include <glib.h>
#include "ccl_oclversions.h"
#include "ccl_device_wrapper.h"
#include "ccl_platforms.h"

/**
 * @defgroup CCL_DEVICE_SELECTOR Device selector
 *
 * This module offers a mechanism for selecting OpenCL devices, mainly
 * for context creation, although its functionality can be used for
 * other purposes.
 *
 * The ::ccl_context_new_from_filters_full() context wrapper constructor
 * (and the ::ccl_context_new_from_filters() macro) accepts a
 * ::CCLDevSelFilters object containing a set of filters. These filters
 * define which devices can be used in the context. Instances of
 * ::CCLDevSelFilters must be initialized to NULL:
 *
 * @dontinclude device_filter.c
 * @skipline cf4ocl objects
 * @until CCLDevSelDevices devices
 *
 * Filters can then be added to the ::CCLDevSelFilters object with
 * the ::ccl_devsel_add_dep_filter() and ::ccl_devsel_add_indep_filter()
 * functions, which add @ref CCL_DEVICE_SELECTOR_DEP_FILTERS "dependent" or
 * @ref CCL_DEVICE_SELECTOR_INDEP_FILTERS "independent" filters,
 * respectively. Filters are processed in the order in which they are
 * added. For example, lets add an independent and a dependent filter to
 * the ::CCLDevSelFilters object:
 *
 * @skipline Add indep
 * @until ccl_devsel_add_dep
 *
 * At this stage, the most common use for the ::CCLDevSelFilters object
 * is to pass it to a context wrapper constructor:
 *
 * @skipline Create context
 * @until ctx =
 *
 * @skipline Free context
 * @until ccl_context_destroy
 *
 * However, the ::CCLDevSelFilters object can also be used for explicit
 * device selection:
 *
 * @skipline Perform device selection
 * @until devices =
 *
 * The ::CCLDevSelDevices object represented by `devices` is just a
 * [GLib pointer array](https://developer.gnome.org/glib/stable/glib-Pointer-Arrays.html#GPtrArray),
 * so we have direct access to the list of device wrappers and its size.
 * For example, lets list the filtered devices by name:
 *
 * @skipline List selected
 * @until &err);
 *
 * @skipline printf(
 * @until For
 * @skipline If
 * @until g_ptr_array_free
 *
 * ::CCLDevSelFilters objects are automatically freed and reset to
 * `NULL` when passed to context wrapper constructors or to the
 * ::ccl_devsel_select() function.
 *
 * See the complete example @ref device_filter.c "here".
 *
 * The device selector module also offers two additional helper
 * functions which return and print a list of all OpenCL devices
 * available in the system, respectively:
 *
 * * ::ccl_devsel_get_device_strings()
 * * ::ccl_devsel_print_device_strings()
 *
 * @{
 */

/** A set of device wrappers, used between filtering steps. */
typedef GPtrArray* CCLDevSelDevices;

/**
 * Independent filter function: Abstract function for filtering
 * one OpenCL device at a time.
 *
 * @param[in] device OpenCL device to filter.
 * @param[in] data Filter data, implementation dependent.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return CL_TRUE if filter accepts device, CL_FALSE otherwise.
 */
typedef cl_bool (*ccl_devsel_indep)(
	CCLDevice* device, void *data, GError **err);

/**
 * Dependent filter function: Abstract function for filtering
 * several OpenCL devices depending on the available device choices.
 *
 * @param[in] devices OpenCL devices to filter.
 * @param[in] data Filter data, implementation dependent.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The OpenCL devices which were accepted by the filter.
 */
typedef CCLDevSelDevices (*ccl_devsel_dep)(
	CCLDevSelDevices devices, void *data, GError **err);

/**
 * A set of independent and dependent device filters.
 *
 * Use the ccl_devsel_add_indep_filter() function to add independent
 * filters and the ccl_devsel_add_dep_filter() function to add dependent
 * device filters.
 *
 * This object should be initialized to NULL:
 *
 *     CCLDevSelFilters filters = NULL;
 *
 * And its location should be passed to the ccl_devsel_add_*_filter()
 * functions:
 *
 *     ccl_devsel_add_indep_filter(&filters, ccl_devsel_indep_type_cpu, NULL);
 *
 * Filters are processed in the order they are added to the set.
 * */
typedef GPtrArray* CCLDevSelFilters;

/* Returns a NULL-terminated array of strings, each one
 * containing the name and vendor of each device in the system. */
CCL_EXPORT
gchar** ccl_devsel_get_device_strings(GError** err);

/* Print to stdout a device description string for each device
 * in the system. */
CCL_EXPORT
void ccl_devsel_print_device_strings(GError** err);

/* Add a independent device filter to the filter set. */
CCL_EXPORT
void ccl_devsel_add_indep_filter(
	CCLDevSelFilters* filters, ccl_devsel_indep filter, void* data);

/* Add a dependent device filter to the filter set. */
CCL_EXPORT
void ccl_devsel_add_dep_filter(
	CCLDevSelFilters* filters, ccl_devsel_dep filter, void* data);

/* Select one or more OpenCL devices based on the provided
 * filters.  */
CCL_EXPORT
CCLDevSelDevices ccl_devsel_select(
	CCLDevSelFilters* filters, GError **err);

/**
 * @defgroup CCL_DEVICE_SELECTOR_INDEP_FILTERS Independent filters
 *
 * Independent filters perform device selection based on per-device
 * characteristics. Devices can be selected by type, name, or using
 * whatever information can be obtained with the device
 * @ref ug_getinfo "info macros". In practice, independent filters are
 * functions which implement the ::ccl_devsel_indep prototype. Functions
 * of this type accept a device wrapper and optional user data,
 * returning `CL_TRUE` if the device is accepted by the filter, or
 * `CL_FALSE` otherwise.
 *
 * _cf4ocl_ includes several independent filters (e.g. filter by type,
 * by name or by platform), but more can be easily implemented and used
 * in the @ref CCL_DEVICE_SELECTOR "device selection" mechanism.
 *
 * @{
 */

/* Independent filter function which accepts devices of the type
 * given in the data parameter. */
CCL_EXPORT
cl_bool ccl_devsel_indep_type(
	CCLDevice* device, void* data, GError **err);

/* Independent filter function which only accepts GPU devices. */
CCL_EXPORT
cl_bool ccl_devsel_indep_type_gpu(
	CCLDevice* device, void *data, GError **err);

/* Independent filter function which only accepts CPU devices. */
CCL_EXPORT
cl_bool ccl_devsel_indep_type_cpu(
	CCLDevice* device, void *data, GError **err);

/* Independent filter function which only accepts accelerator
 * devices. */
CCL_EXPORT
cl_bool ccl_devsel_indep_type_accel(
	CCLDevice* device, void *data, GError **err);

/* Independent filter which selects devices based on device
 * name, device vendor and/or platform name. */
CCL_EXPORT
cl_bool ccl_devsel_indep_string(
	CCLDevice* dev, void *data, GError **err);

/* Independent filter function which only accepts devices of a
 * specified platform. */
CCL_EXPORT
cl_bool ccl_devsel_indep_platform(
	CCLDevice* device, void *data, GError **err);

/** @} */

/**
 * @defgroup CCL_DEVICE_SELECTOR_DEP_FILTERS Dependent filters
 *
 * Dependent filters perform device selection based on the
 * characteristics of all devices present in the list passed to them.
 * In practice, dependent filters are functions which implement the
 * ::ccl_devsel_dep prototype. Functions of this type accept a list of
 * device wrappers and optional user data, returning a new list with
 * the devices present in the original list and which were accepted
 * by the filter.
 *
 * _cf4ocl_ includes several dependent filters (e.g. a filter to select
 * devices which belong to the platform associated with the first
 * device in the list, and a filter to select devices using a menu).
 * Other dependent filters can be implemented by client code and used
 * in the @ref CCL_DEVICE_SELECTOR "device selection" mechanism.
 *
 * @{
 */

/* Dependent filter function which only accepts devices of the
 * same platform (the platform to which the first device belong to). */
CCL_EXPORT
CCLDevSelDevices ccl_devsel_dep_platform(
	CCLDevSelDevices devices, void *data, GError **err);

/* Dependent filter function which presents a menu to the user
 * allowing him to select the desired device. */
CCL_EXPORT
CCLDevSelDevices ccl_devsel_dep_menu(
	CCLDevSelDevices devices, void *data, GError **err);

/** @} */

/** @} */

#endif
