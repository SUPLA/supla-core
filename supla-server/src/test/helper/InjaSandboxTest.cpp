/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "InjaSandboxTest.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <stdexcept>
#include <string>

namespace testing {

using std::runtime_error;

InjaSandboxTest::InjaSandboxTest(void) {}

InjaSandboxTest::~InjaSandboxTest(void) {}

TEST_F(InjaSandboxTest, includeStatementsAreNotAllowed) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render("{% include \"content.html\" %}", data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("Include statements are not allowed."), e.what());
  }
}

TEST_F(InjaSandboxTest, extendsStatementsAreNotAllowed) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render("{% extends \"content.html\" %}", data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("Extends statements are not allowed."), e.what());
  }
}

TEST_F(InjaSandboxTest, forLoopssAreNotAllowed) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render(
        "{% for i in range(4) %}{{ loop.index1 }}{% endfor %}", data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("For loops are not allowed."), e.what());
  }
}

TEST_F(InjaSandboxTest, tooManyControlBlocksInTemplate) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render(
        "{% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% {% ",
        data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("Too many control blocks in template."), e.what());
  }
}

TEST_F(InjaSandboxTest, tooManyIfStatementsInTemplate) {
  nlohmann::json data;
  try {
    sandbox.validate_and_render(
        "{% if {% if {% if {% if {% if {% if {% if {% if {% if {% if {% if {% "
        "if {% if {% if {% if {% if ",
        data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("Too many if statements in template."), e.what());
  }
}

TEST_F(InjaSandboxTest, exceededOutputSize) {
  nlohmann::json data;

  std::string text;
  text.reserve(1025);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 9);

  for (std::size_t i = 0; i < 1025; ++i) {
    text += static_cast<char>('0' + dis(gen));
  }

  data["text"] = text;
  try {
    sandbox.validate_and_render("{{ text }}", data);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_EQ(std::string("Template execution exceeded allowed output size."),
              e.what());
  }
}

} /* namespace testing */
