import os


def config_log(path, max_level, filename):
    return {
            'version': 1,
            'disable_existing_loggers': False,
            'formatters': {
                    'standard': {
                            'format': '%(asctime)s [%(levelname)s] %(name)s: %(message)s'
                    }
            },
            'handlers': {
                    'default': {
                            'level': max_level,
                            'class': 'logging.FileHandler',
                            'formatter': 'standard',
                            'filename': os.path.join(*path, filename),
                            'mode': 'w'
                    }
            },
            'loggers': {
                    '': {
                            'handlers': ['default'],
                            'level': max_level,
                            'propagate': True
                    }
            }
    }
