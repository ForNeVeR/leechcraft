/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "temp2rgbtest.h"
#include <QtTest>
#include "../colortemp.cpp"

QTEST_APPLESS_MAIN (LeechCraft::Poshuku::DCAC::Temp2RgbTest)

namespace LeechCraft
{
namespace Poshuku
{
namespace DCAC
{
	namespace
	{
#if QT_VERSION >= 0x050500
		struct ColorPrinter
		{
			QRgb Rgb_;
			ColorPrinter (QRgb rgb)
			: Rgb_ { rgb }
			{
			}
		};

		bool operator== (const ColorPrinter& left, const ColorPrinter& right)
		{
			return left.Rgb_ == right.Rgb_;
		}

		char* toString (const ColorPrinter& rgb)
		{
			using QTest::toString;
			return toString ("RGB { " +
					QByteArray::number (qRed (rgb.Rgb_)) + ", " +
					QByteArray::number (qGreen (rgb.Rgb_)) + ", " +
					QByteArray::number (qBlue (rgb.Rgb_)) + "}");
		}
#else
		using ColorPrinter = QRgb;
#endif
	}

	void Temp2RgbTest::testTemp2Rgb_data ()
	{
		QTest::addColumn<uint> ("temp");
		QTest::addColumn<uint> ("result");

#define ADD(x,r,g,b) QTest::newRow (#x) << static_cast<uint> (x) << qRgb(r,g,b);
		ADD (4000, 255, 206, 166);
		ADD (4500, 255, 218, 187);
		ADD (5000, 255, 228, 206);
		ADD (5500, 255, 237, 222);
		ADD (6000, 255, 246, 237);
		ADD (6500, 255, 254, 250);
		ADD (7000, 243, 242, 255);
#undef ADD
	}

	void Temp2RgbTest::testTemp2Rgb ()
	{
		QFETCH (uint, temp);
		QFETCH (uint, result);

		QCOMPARE (ColorPrinter { Temp2Rgb (temp) }, ColorPrinter { result });
	}
}
}
}
