/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <stdexcept>
#include <thread>

#ifndef Q_MOC_RUN // see https://bugreports.qt-project.org/browse/QTBUG-22829
#include <boost/program_options.hpp>
#endif

#include <QApplication>
#include "appinfo.h"
#include "gdblauncher.h"
#include "crashdialog.h"

namespace CrashProcess = LeechCraft::AnHero::CrashProcess;

namespace
{
	namespace bpo = boost::program_options;

	CrashProcess::AppInfo ParseOptions (int argc, char **argv)
	{
		bpo::options_description desc ("Known options");
		desc.add_options ()
				("signal", bpo::value<int> (), "the signal that triggered the crash handler")
				("pid", bpo::value<uint64_t> ()->required (), "the PID of the crashed process")
				("path", bpo::value<std::string> ()->required (), "the application path of the crashed process")
				("version", bpo::value<std::string> ()->required (), "the LeechCraft version at the moment of the crash")
				("cmdline", bpo::value<std::string> (), "the command line LeechCraft was started with");

		bpo::command_line_parser parser (argc, argv);
		bpo::variables_map vm;
		bpo::store (parser
				.options (desc)
				.allow_unregistered ()
				.run (), vm);
		bpo::notify (vm);

		if (!vm.count ("pid"))
			throw std::runtime_error ("PID parameter not set");

		return
		{
			vm ["signal"].as<int> (),
			vm ["pid"].as<uint64_t> (),
			QString::fromUtf8 (vm ["path"].as<std::string> ().c_str ()),
			vm ["version"].as<std::string> ().c_str (),
			vm ["cmdline"].as<std::string> ().c_str ()
		};
	}
}

int main (int argc, char **argv)
{
	QApplication app (argc, argv);

	const auto& info = ParseOptions (argc, argv);

	auto l = new CrashProcess::GDBLauncher (info.PID_, info.Path_);
	auto dia = new CrashProcess::CrashDialog (info);
	QObject::connect (l,
			SIGNAL (gotOutput (QString)),
			dia,
			SLOT (appendTrace (QString)));

	return app.exec ();
}
