import React, { useState, useEffect, useRef } from 'react';
import './App.css';

const API_URL = 'http://localhost:8080';

function App() {
    const [editorState, setEditorState] = useState({ text: '', cursor: 0 });
    const [searchPattern, setSearchPattern] = useState('');
    const [highlightedPositions, setHighlightedPositions] = useState([]);
    const editorRef = useRef(null);

    const fetchData = async () => {
        try {
            const response = await fetch(`${API_URL}/state`);
            const data = await response.json();
            setEditorState(data);
        } catch (error) {
            console.error("Error fetching state:", error);
        }
    };

    useEffect(() => {
        fetchData();
    }, []);

    const handleApiCall = async (endpoint, body = {}) => {
        try {
            const response = await fetch(`${API_URL}/${endpoint}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(body),
            });
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            const data = await response.json();
            if (data && data.text !== undefined) {
                setEditorState(data);
            }
            return data; // Return data for search
        } catch (error) {
            console.error(`Error with ${endpoint}:`, error);
        }
    };

    const handleInsert = (char) => {
        handleApiCall('insert', { char });
    };

    const handleBackspace = () => {
        handleApiCall('backspace');
    };

    const handleUndo = () => {
        handleApiCall('undo');
    };

    const handleRedo = () => {
        handleApiCall('redo');
    };

    const handleSearch = async (pattern) => {
        setSearchPattern(pattern);
        if (pattern) {
            const data = await handleApiCall('search', { pattern });
            if (data && data.positions) {
                setHighlightedPositions(data.positions);
            }
        } else {
            setHighlightedPositions([]);
        }
    };

    const handleKeyPress = (e) => {
        e.preventDefault();
        if (e.key.length === 1) {
            handleInsert(e.key);
        } else if (e.key === 'Backspace') {
            handleBackspace();
        } else if (e.key === 'ArrowLeft') {
            handleApiCall('move-left');
        } else if (e.key === 'ArrowRight') {
            handleApiCall('move-right');
        }
    };

    const focusEditor = () => {
        editorRef.current.focus();
    };

    const isHighlighted = (index) => {
        for (const pos of highlightedPositions) {
            if (index >= pos && index < pos + searchPattern.length) {
                return true;
            }
        }
        return false;
    };

    return (
        <div className="App">
            <h1>Smart Text Editor</h1>
            <div className="search-container">
                <input
                    type="text"
                    placeholder="Search..."
                    value={searchPattern}
                    onChange={(e) => handleSearch(e.target.value)}
                />
            </div>
            <div className="editor-container" onClick={focusEditor}>
                <div 
                    className="editor-area" 
                    onKeyDown={handleKeyPress} 
                    tabIndex={0}
                    ref={editorRef}
                >
                    {editorState && editorState.text ? editorState.text.split('').map((char, index) => (
                        <span 
                            key={index} 
                            className={`${index === editorState.cursor ? 'cursor-char' : ''} ${isHighlighted(index) ? 'highlight' : ''}`}
                        >
                            {char === ' ' ? '\u00A0' : char}
                        </span>
                    )) : null}
                    {editorState && editorState.text && editorState.cursor === editorState.text.length && <span className="cursor-end"></span>}
                </div>
            </div>
            <div className="controls">
                <button onClick={handleUndo}>Undo</button>
                <button onClick={handleRedo}>Redo</button>
            </div>
            <div className="instructions">
                <p>Click on the editor area to start typing.</p>
            </div>
        </div>
    );
}

export default App;
