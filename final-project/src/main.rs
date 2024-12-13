use std::collections::HashMap;

const BLOCK_SIZE: usize = 4096;
const NUM_DIRECT_POINTERS: usize = 10;

#[derive(Clone, Debug)]
enum FileType {
    RegularFile,
    Directory,
}

// Inode Structure
#[derive(Clone, Debug)]
struct Inode {
    id: u64,
    name: String,
    size: u64,
    file_type: FileType,
    direct_pointers: [Option<u64>; 10],
    entries: Option<Vec<u64>>, // For directories only
}

// Journal Entry
struct JournalEntry {
    operation: String,
    committed: bool,
}
// Journal Structure
struct Journal {
    entries: Vec<JournalEntry>,
}

impl Journal {
    fn new() -> Self {
        return Self {
            entries: Vec::new(),
        };
    }

    fn add_entry(&mut self, operation: String) {
        self.entries.push(JournalEntry {
            operation,
            committed: true,
        });
    }

    fn undo(&mut self) -> Option<String> {
        return self.entries.pop().map(|entry| entry.operation);
    }

    fn print_journal(&mut self) {
        println!("Journal Entries");
        for (i, entry) in self.entries.iter().enumerate() {
            println!(
                "  {}. {} [Committed: {}]",
                i + 1,
                entry.operation,
                entry.committed
            );
        }
    }
}

struct FileSystem {
    inodes: HashMap<u64, Inode>,
    blocks: HashMap<u64, Vec<u8>>,
    journal: Journal,
    next_inode_id: u64,
}

impl FileSystem {
    fn new() -> Self {
        return Self {
            inodes: HashMap::new(),
            blocks: HashMap::new(),
            journal: Journal::new(),
            next_inode_id: 1,
        };
    }

    fn create_directory(&mut self, name: &str) -> u64 {
        let id = self.next_inode_id;
        self.next_inode_id += 1;

        let dir = Inode {
            id,
            name: name.to_string(),
            size: 0,
            file_type: FileType::Directory,
            direct_pointers: [None; NUM_DIRECT_POINTERS],
            entries: Some(Vec::new()),
        };

        self.inodes.insert(id, dir);
        self.journal
            .add_entry(format!("CREATE DIRECTORY: {}", name));
        return id;
    }

    fn create_file(&mut self, name: &str) -> u64 {
        let id = self.next_inode_id;
        self.next_inode_id += 1;

        let file = Inode {
            id,
            name: name.to_string(),
            size: 0,
            file_type: FileType::RegularFile,
            direct_pointers: [None; NUM_DIRECT_POINTERS],
            entries: None,
        };
        self.inodes.insert(id, file);
        self.journal.add_entry(format!("CREATE FILE: {}", name));
        return id;
    }

    fn add_file_to_directory(&mut self, file_id: u64, dir_id: u64) {
        if let Some(dir_inode) = self.inodes.get_mut(&dir_id) {
            if let Some(entries) = dir_inode.entries.as_mut() {
                entries.push(file_id);
                self.journal
                    .add_entry(format!("ADD FILE: {} TO DIRECTORY: {}", file_id, dir_id))
            }
        }
    }

    fn write_to_file(&mut self, file_id: u64, data: &[u8]) {
        if let Some(file_inode) = self.inodes.get_mut(&file_id) {
            let mut blocks_needed = (data.len() + BLOCK_SIZE - 1) / BLOCK_SIZE;
            let mut data_offset = 0;

            for i in 0..NUM_DIRECT_POINTERS {
                if blocks_needed == 0 {
                    break;
                }

                let block_id = self.next_inode_id;
                self.next_inode_id += 1;

                let end_offset = (data_offset + BLOCK_SIZE).min(data.len());
                let block_data = data[data_offset..end_offset].to_vec();

                // Write to the block storage
                self.blocks.insert(block_id, block_data);
                file_inode.direct_pointers[i] = Some(block_id);

                blocks_needed -= 1;
                data_offset = end_offset;
            }
            file_inode.size = data.len() as u64;
        }
    }

    fn read_file(&self, file_id: u64) -> Vec<u8> {
        if let Some(file_inode) = self.inodes.get(&file_id) {
            let mut data = Vec::new();
            for pointer in file_inode.direct_pointers.iter() {
                if let Some(block_id) = pointer {
                    if let Some(block_data) = self.blocks.get(&block_id) {
                        data.extend_from_slice(block_data);
                    } else {
                        eprintln!("Warning: Block {} not found for file {}", block_id, file_id);
                    }
                }
            }
            return data;
        }
        return Vec::new();
    }

    fn list_directories_and_files(&self) {
        for inode in self.inodes.values() {
            match inode.file_type {
                FileType::Directory => {
                    println!("Directory {} (ID: {}):", inode.name, inode.id);
                    if let Some(entries) = &inode.entries {
                        for &entry_id in entries {
                            if let Some(entry) = self.inodes.get(&entry_id) {
                                println!(
                                    " - File {} (ID: {}, Size: {} bytes)",
                                    entry.name, entry.id, entry.size
                                );
                            }
                        }
                    }
                }
                _ => {}
            }
        }
    }
}

fn main() {
    let mut fs = FileSystem::new();

    // Create directories
    let dir1 = fs.create_directory("Documents");
    let dir2 = fs.create_directory("Pictures");

    // Create files
    let file1 = fs.create_file("doc1.txt");
    let file2 = fs.create_file("doc2.txt");
    let file3 = fs.create_file("pic1.jpg");

    // Add files to directories
    fs.add_file_to_directory(file1, dir1);
    fs.add_file_to_directory(file2, dir1);
    fs.add_file_to_directory(file3, dir2);

    // Write to file
    fs.write_to_file(file1, b"Hello, World!");

    // List directories and files
    println!("\n=== Directory Listing ===");
    fs.list_directories_and_files();

    // Read from file
    let data = fs.read_file(file1);
    println!("\n=== Read File ===");
    println!("File Data: {}", String::from_utf8_lossy(&data),);

    // Print journal
    println!("\n=== Journal ===");
    fs.journal.print_journal();

    // Undo the last operation
    println!("\n=== Undo Operation ===");
    if let Some(undone_operation) = fs.journal.undo() {
        println!("Undid operation: {}", undone_operation);
    } else {
        println!("Nothing to undo");
    }

    println!("\n=== Final Journal ===");
    fs.journal.print_journal();
}
