import sys
import datetime

sys.path.append('build/')

import cache_module
cache = cache_module.Cache(datetime.timedelta(seconds=10))
cache.put("key", {"value": 123})
print(cache.get("key"))  # {'value': 123}
