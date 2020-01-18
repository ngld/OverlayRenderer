import sys

deps = {
    'boost': {
        'url': 'https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.gz',
        'dest': 'third_party/boost',
        'strip': 1,
        'hash': ['sha256', '1b89056d901916123509f01377a9ce4fa747bbe474ea06dbd41433a49763ccd6'],
    },
}

if sys.platform == 'win32':
    deps.update({
        'cef': {
            'url': 'http://opensource.spotify.com/cefbuilds/cef_binary_75.1.14%2Bgc81164e%2Bchromium-75.0.3770.100_windows64.tar.bz2',
            'dest': 'third_party/cef',
            'strip': 1,
            'hash': ['sha256', '898f03be7af2b48e896576469ceae9de2e4188ed3784629d75fe401d4cd00b23'],
        },
    })
elif sys.platform == 'linux':
    deps.update({
        'cef': {
            'url': 'http://opensource.spotify.com/cefbuilds/cef_binary_75.1.14%2Bgc81164e%2Bchromium-75.0.3770.100_linux64.tar.bz2',
            'dest': 'third_party/cef',
            'strip': 1,
            'hash': ['sha256', 'GHI'],
        },
    })
else:
    raise Exception('Unsupported platform!')
