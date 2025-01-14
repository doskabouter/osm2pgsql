/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This file is part of osm2pgsql (https://osm2pgsql.org/).
 *
 * Copyright (C) 2006-2021 by the osm2pgsql developer community.
 * For a full list of authors see the git log.
 */

#include "config.h"

#include "db-copy.hpp"
#include "format.hpp"
#include "output-gazetteer.hpp"
#include "output-null.hpp"
#include "output-pgsql.hpp"
#include "output.hpp"
#include "taginfo-impl.hpp"

#ifdef HAVE_LUA
# include "output-flex.hpp"
# define flex_backend "flex, "
#else
# define flex_backend ""
#endif

#include <cstring>
#include <stdexcept>

std::shared_ptr<output_t>
output_t::create_output(std::shared_ptr<middle_query_t> const &mid,
                        options_t const &options)
{
    auto copy_thread =
        std::make_shared<db_copy_thread_t>(options.database_options.conninfo());

    if (options.output_backend == "pgsql") {
        return std::make_shared<output_pgsql_t>(mid, options, copy_thread);

#ifdef HAVE_LUA
    } else if (options.output_backend == "flex") {
        return std::make_shared<output_flex_t>(mid, options, copy_thread);
#endif

    } else if (options.output_backend == "gazetteer") {
        return std::make_shared<output_gazetteer_t>(mid, options, copy_thread);

    } else if (options.output_backend == "null") {
        return std::make_shared<output_null_t>(mid, options);
    }

    throw std::runtime_error{
        "Output backend '{}' not recognised. Should be one "
        "of [pgsql, " flex_backend
        "gazetteer, null]."_format(options.output_backend)};
}

output_t::output_t(std::shared_ptr<middle_query_t> const &mid,
                   options_t const &options)
: m_mid(mid), m_options(options)
{}

output_t::~output_t() = default;

options_t const *output_t::get_options() const { return &m_options; }

void output_t::merge_expire_trees(output_t *) {}
