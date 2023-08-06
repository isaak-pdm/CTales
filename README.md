# CTales

You could call this a minimum viable interactive fiction game engine. It uses a JSON for a simple CYOA experience and ncurses to display it.

## Prerequisites

Install these libraries:

- ncurses
- jansson
- mimalloc

### Debian

```bash
sudo apt-get update && sudo apt-get install libncurses5-dev libjansson-dev libmimalloc-dev cmake
```

### Arch Linux

```bash
sudo pacman -Sy && sudo pacman -S ncurses jansson mimalloc cmake
```


## Compile and run

1. Clone and navigate to the repository:

    ```bash
    git clone https://github.com/isaak-pdm/CTales && cd CTales
    ```

2. Create a build directory, navigate to it, and run CMake:

    ```bash
    mkdir build && cd build && cmake ..
    ```

3. Compile and run the game:

    ```bash
    make && ./CTales <path_to_your_json_file>
    ```

*Replace `<path_to_your_json_file>` with your game state JSON file path.*

## Game JSON Structure

Each passage is a JSON object within an array representing the game state. Each passage includes `PassageName`, `Content`, and `Links`.

```json
[
  {
    "PassageName": "passage1",
    "Content": "This is the first passage.",
    "Links": [
      {
        "Option": "Go to passage 2",
        "Target": "passage2"
      },
      {
        "Option": "Go to passage 3",
        "Target": "passage3"
      }
    ]
  }, 
  ...
]
```

## Contribute

Fork the repository, use a feature branch, and warmly welcome pull requests.

## Licence

MIT License - see `LICENSE` file.

## Author

Isaak - Initial commit. Reach me via GitHub.