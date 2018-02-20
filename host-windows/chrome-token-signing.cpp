/*
 * Chrome Token Signing Native Host
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "BinaryUtils.h"
#include "CertificateSelector.h"
#include "ContextMaintainer.h"
#include "Exceptions.h"
#include "IOCommunicator.h"
#include "jsonxx.h"
#include "Labels.h"
#include "Logger.h"
#include "Signer.h"
#include "VersionInfo.h"

#include <memory>

using namespace std;
using namespace jsonxx;

int main(int argc, char **argv)
{
	IOCommunicator ioCommunicator;

	while (true)
	{
		_log("Parsing input...");
		jsonxx::Object jsonRequest, jsonResponse;
		try {
			if (!jsonRequest.parse(ioCommunicator.readMessage()))
				throw InvalidArgumentException();
			if (!jsonRequest.has<string>("type") || !jsonRequest.has<string>("nonce") || !jsonRequest.has<string>("origin"))
				throw InvalidArgumentException();
			if (!ContextMaintainer::isSameOrigin(jsonRequest.get<string>("origin")))
				throw InconsistentOriginException();

			if (jsonRequest.has<string>("lang"))
				Labels::l10n.setLanguage(jsonRequest.get<string>("lang"));

			string type = jsonRequest.get<string>("type");
			if (type == "VERSION")
				jsonResponse << "version" << VERSION;
			else if (jsonRequest.get<string>("origin").compare(0, 6, "https:"))
				throw NotAllowedException("Origin doesn't contain https");
			else if (type == "CERT")
			{
				unique_ptr<CertificateSelector> certificateSelector(CertificateSelector::createCertificateSelector());
				vector<unsigned char> selectedCert = certificateSelector->getCert(!jsonRequest.has<string>("filter") || jsonRequest.get<string>("filter") != "AUTH");
				ContextMaintainer::saveCertificate(selectedCert);
				jsonResponse << "cert" << BinaryUtils::bin2hex(selectedCert);
			}
			else if (type == "SIGN")
			{
				if (!jsonRequest.has<string>("cert") || !jsonRequest.has<string>("hash"))
					throw InvalidArgumentException();

				vector<unsigned char> cert = BinaryUtils::hex2bin(jsonRequest.get<string>("cert"));
				vector<unsigned char> digest = BinaryUtils::hex2bin(jsonRequest.get<string>("hash"));
				_log("signing hash: %s, with certId: %s", jsonRequest.get<string>("hash").c_str(), jsonRequest.get<string>("cert").c_str());
				if (!ContextMaintainer::isSelectedCertificate(cert))
					throw NotSelectedCertificateException();

				switch (digest.size())
				{
				case BINARY_SHA1_LENGTH:
				case BINARY_SHA224_LENGTH:
				case BINARY_SHA256_LENGTH:
				case BINARY_SHA384_LENGTH:
				case BINARY_SHA512_LENGTH:
					break;
				default:
					_log("Hash length %i is invalid", digest.size());
					throw InvalidHashException();
				}
				unique_ptr<Signer> signer(Signer::createSigner(cert));

				if (!signer->showInfo(jsonRequest.get<string>("info", string())))
					throw UserCancelledException();

				jsonResponse << "signature" << BinaryUtils::bin2hex(signer->sign(digest));
			}
			else
				throw InvalidArgumentException();
		}
		// Only catch terminating exceptions here
		catch (const InvalidArgumentException &e)
		{
			_log("Handling exception: %s", e.getErrorCode());
			ioCommunicator.sendMessage((Object() << "result" << e.getErrorCode() << "message" << e.what()).json());
			return EXIT_FAILURE;
		}
		catch (const BaseException &e) {
			jsonResponse << "result" << e.getErrorCode() << "message" << e.what();
		}

		if (jsonRequest.has<string>("nonce"))
			jsonResponse << "nonce" << jsonRequest.get<string>("nonce");
		if (!jsonResponse.has<string>("result"))
			jsonResponse << "result" << "ok";
		jsonResponse << "api" << API;
		ioCommunicator.sendMessage(jsonResponse.json());
	}
	return EXIT_SUCCESS;
}
