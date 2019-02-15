/*
 * Copyright (c) 2018, Amy de Buitléir All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "cppqc.h"

#include <random>
#include <sstream>

class Rectangle {
public:
  Rectangle (int w, int h) {
    width = w;
    height = h;
  }
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  int getArea() const { return width*height; }
  friend std::ostream& operator << (std::ostream& os, const Rectangle& r) {
    os << "width: " << r.width << " height: " << r.height << std::endl;
    return os ;
  }
private:
  int width;
  int height;
};

class CustomGenerator {
public:
  Rectangle unGen(cppqc::RngEngine &rng, std::size_t n) {
    std::uniform_int_distribution<> dist{1, static_cast<int>(n) + 1};
    int w = dist(rng);
    int h = (n + 1)/w;
    return Rectangle(w, h);
  }

  std::vector<Rectangle> shrink(const Rectangle &r) {
    std::vector<Rectangle> ret;
    if (r.getWidth() > 1)
      ret.push_back(Rectangle(r.getWidth()-1, r.getHeight()));
    if (r.getHeight() > 1) {
      ret.push_back(Rectangle(r.getWidth(), r.getHeight()-1));
    }
    return ret;
  }
};

// A silly test just to demonstrate the custom generator.
struct PropTestCustomGen : cppqc::Property<Rectangle>
{
  PropTestCustomGen() : Property(CustomGenerator()) {}
    bool check(const Rectangle &r) const override
    {
      return (r.getArea() == r.getWidth() * r.getHeight());
    }
    std::string name() const override
    {
        return "TestCustomGen";
    }
};

int main()
{
  cppqc::quickCheckOutput(PropTestCustomGen());
}
