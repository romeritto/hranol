#include "FolderCrawler.h"
#include "ImageStore.h"
#include "HranolException.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace std;

int main() {
	vector<string> halo{ "Run 13-56-55", "hranol.exe" };
	FolderCrawler c(halo, "(.*)\.png", true, true);

	while (c.has_next_run()) {
		unique_ptr<IImageStore> p(nullptr);

		try {
			p = c.get_next_run();
			cout << p->size() << endl;
			if (p->size() == 0)
				continue;

			p->set_dest(p->get_origin() / "cecky");
			for (int i = 0; i < p->size(); ++i) {
				auto m = p->get(i);
				p->save(i);
				p->release(i);
			}
		}
		catch (const HranolDirectoryDNE &e) {
			cerr << e.what() << endl;
			continue;
		}
		catch (const HranolIndexOutOfRange &e) {
			cerr << e.what() << endl;
			continue;
		}
		catch (const HranolRuntimeException &e) {
			cerr << e.what() << endl;
			continue;
		}
	}

	return 0;
}