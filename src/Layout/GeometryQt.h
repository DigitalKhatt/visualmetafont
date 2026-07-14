#pragma once

#include <string>

#include <QPainterPath>

#include "digitalkhatt/geometry/geometry.h"

// Qt-side reimplementation of the pieces of geometry::GeometrySet that need
// Qt types (QPainterPath). The Qt-free geometry library itself
// (lib/digitalkhatt) has no Qt dependency; this is the adapter for code in
// src/Layout that wants to draw a GeometrySet.

QPainterPath toQPainterPath(const geometry::GeometrySet& geomSet);

// Manual debugging aid: opens a modal dialog visualizing the contact between
// two glyph outlines (nearest points, contact normal, and the touching
// polygon pair if any). Not wired into the solver's hot path — call it by
// hand from a debugger or a temporary breakpoint when investigating a
// specific contact.
void debugDistance(const geometry::GeometrySet& A, const geometry::GeometrySet& B,
                   const std::string& nameA, const std::string& nameB,
                   const geometry::GSContact& gsContact);
